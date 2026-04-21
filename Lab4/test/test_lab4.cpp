#include <gtest/gtest.h>
#include <windows.h>
#include <cstring>

#include "Lab4.h"

// ─── Вспомогательный класс: временный файл очереди ───────────────────────

struct TempQueue
{
    const char* name;
    HANDLE hFile     = INVALID_HANDLE_VALUE;
    HANDLE hSemEmpty = NULL;
    HANDLE hSemFull  = NULL;
    HANDLE hMutex    = NULL;
    SharedData sd{};

    explicit TempQueue(int capacity, const char* fname = "test_q.bin")
        : name(fname)
    {
        hFile = CreateFileA(name,
            GENERIC_READ | GENERIC_WRITE,
            FILE_SHARE_READ | FILE_SHARE_WRITE,
            NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
       /* ASSERT_NE(hFile, INVALID_HANDLE_VALUE);
        ASSERT_TRUE(CreateQueueFile(hFile, capacity));*/

        hSemEmpty = CreateSemaphoreA(NULL, capacity, capacity, NULL);
        hSemFull  = CreateSemaphoreA(NULL, 0,        capacity, NULL);
        hMutex    = CreateMutexA    (NULL, FALSE,               NULL);
        //ASSERT_NE(hSemEmpty, (HANDLE)NULL);
        //ASSERT_NE(hSemFull,  (HANDLE)NULL);
        //ASSERT_NE(hMutex,    (HANDLE)NULL);

        sd = { hSemEmpty, hSemFull, hMutex, hFile, capacity };
    }

    ~TempQueue()
    {
        if (hFile     != INVALID_HANDLE_VALUE) CloseHandle(hFile);
        if (hSemEmpty) CloseHandle(hSemEmpty);
        if (hSemFull)  CloseHandle(hSemFull);
        if (hMutex)    CloseHandle(hMutex);
        DeleteFileA(name);
    }
};

// ─── Тесты констант и структур ───────────────────────────────────────────

TEST(StructTest, RecordSizeEqualsMaxLen)
{
    EXPECT_EQ(sizeof(MessageRecord), (size_t)MSG_MAX_LEN);
}

TEST(StructTest, HeaderFitsInOneSlot)
{
    EXPECT_LE(sizeof(QueueHeader), (size_t)MSG_MAX_LEN);
}

TEST(StructTest, SlotOffsetIsCorrect)
{
    EXPECT_EQ(SlotOffset(0), (long long)MSG_MAX_LEN);
    EXPECT_EQ(SlotOffset(1), (long long)MSG_MAX_LEN * 2);
    EXPECT_EQ(SlotOffset(3), (long long)MSG_MAX_LEN * 4);
}

// ─── Тесты объектов синхронизации ────────────────────────────────────────

TEST(SyncTest, SemaphoresCreated)
{
    HANDLE hEmpty = CreateSemaphoreA(NULL, 5, 5, NULL);
    HANDLE hFull  = CreateSemaphoreA(NULL, 0, 5, NULL);
    HANDLE hMtx   = CreateMutexA(NULL, FALSE, NULL);

    EXPECT_NE(hEmpty, (HANDLE)NULL);
    EXPECT_NE(hFull,  (HANDLE)NULL);
    EXPECT_NE(hMtx,   (HANDLE)NULL);

    CloseHandle(hEmpty);
    CloseHandle(hFull);
    CloseHandle(hMtx);
}

TEST(SyncTest, EmptySemaphoreBlocksImmediately)
{
    HANDLE hFull = CreateSemaphoreA(NULL, 0, 4, NULL);
    ASSERT_NE(hFull, (HANDLE)NULL);

    DWORD res = WaitForSingleObject(hFull, 0);
    EXPECT_EQ(res, (DWORD)WAIT_TIMEOUT);

    CloseHandle(hFull);
}

TEST(SyncTest, FullSemaphoreBlocksImmediately)
{
    HANDLE hEmpty = CreateSemaphoreA(NULL, 0, 4, NULL);
    ASSERT_NE(hEmpty, (HANDLE)NULL);

    DWORD res = WaitForSingleObject(hEmpty, 0);
    EXPECT_EQ(res, (DWORD)WAIT_TIMEOUT);

    CloseHandle(hEmpty);
}

// ─── Тесты очереди (Enqueue / Dequeue) ───────────────────────────────────

TEST(QueueTest, EnqueueOneMessage)
{
    TempQueue q(4);
    EXPECT_TRUE(Enqueue(q.sd, "hello"));

    // После enqueue hSemFull должен быть = 1
    DWORD res = WaitForSingleObject(q.hSemFull, 0);
    EXPECT_EQ(res, (DWORD)WAIT_OBJECT_0);
    ReleaseSemaphore(q.hSemFull, 1, NULL);
}

TEST(QueueTest, EnqueueThenDequeue)
{
    TempQueue q(4);
    ASSERT_TRUE(Enqueue(q.sd, "world"));

    char buf[MSG_MAX_LEN]{};
    ASSERT_TRUE(Dequeue(q.sd, buf));
    EXPECT_STREQ(buf, "world");
}

TEST(QueueTest, FifoOrder)
{
    TempQueue q(5);
    ASSERT_TRUE(Enqueue(q.sd, "first"));
    ASSERT_TRUE(Enqueue(q.sd, "second"));
    ASSERT_TRUE(Enqueue(q.sd, "third"));

    char buf[MSG_MAX_LEN]{};
    ASSERT_TRUE(Dequeue(q.sd, buf)); EXPECT_STREQ(buf, "first");
    ASSERT_TRUE(Dequeue(q.sd, buf)); EXPECT_STREQ(buf, "second");
    ASSERT_TRUE(Dequeue(q.sd, buf)); EXPECT_STREQ(buf, "third");
}

TEST(QueueTest, WrapAround)
{
    TempQueue q(3);
    Enqueue(q.sd, "a");
    Enqueue(q.sd, "b");

    char buf[MSG_MAX_LEN]{};
    Dequeue(q.sd, buf);
    Dequeue(q.sd, buf);

    ASSERT_TRUE(Enqueue(q.sd, "c"));
    ASSERT_TRUE(Enqueue(q.sd, "d"));
    ASSERT_TRUE(Enqueue(q.sd, "e"));

    ASSERT_TRUE(Dequeue(q.sd, buf)); EXPECT_STREQ(buf, "c");
    ASSERT_TRUE(Dequeue(q.sd, buf)); EXPECT_STREQ(buf, "d");
    ASSERT_TRUE(Dequeue(q.sd, buf)); EXPECT_STREQ(buf, "e");
}

TEST(QueueTest, EmptyMessage)
{
    TempQueue q(2);
    ASSERT_TRUE(Enqueue(q.sd, ""));
    char buf[MSG_MAX_LEN]{};
    ASSERT_TRUE(Dequeue(q.sd, buf));
    EXPECT_STREQ(buf, "");
}

TEST(QueueTest, MaxLengthMessage)
{
    TempQueue q(2);
    char msg[MSG_MAX_LEN]{};
    memset(msg, 'Z', MSG_MAX_LEN - 1);

    ASSERT_TRUE(Enqueue(q.sd, msg));
    char buf[MSG_MAX_LEN]{};
    ASSERT_TRUE(Dequeue(q.sd, buf));
    EXPECT_STREQ(buf, msg);
}
