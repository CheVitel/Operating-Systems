// Lab4Test.cpp
#include <gtest/gtest.h>
#include "../Lab4.h"
#include <windows.h>
#include <cstring>

static HANDLE CreateTestFile(const char* name, int capacity)
{
    HANDLE hFile = CreateFileA(
        name,
        GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        NULL);
    if (hFile == INVALID_HANDLE_VALUE)
        return INVALID_HANDLE_VALUE;

    QueueHeader hdr;
    hdr.capacity = capacity;
    hdr.head = 0;
    hdr.tail = 0;
    WriteHeader(hFile, hdr);

    MsgRecord empty;
    ZeroMemory(&empty, sizeof(MsgRecord));
    for (int i = 0; i < capacity; i++)
        WriteSlot(hFile, i, empty);

    return hFile;
}

TEST(QueueFileTest, HeaderReadWrite)
{
    HANDLE hFile = CreateTestFile("test_header.bin", 5);
    ASSERT_NE(hFile, INVALID_HANDLE_VALUE);

    QueueHeader hdr;
    ASSERT_TRUE(ReadHeader(hFile, hdr));
    EXPECT_EQ(hdr.capacity, 5);
    EXPECT_EQ(hdr.head, 0);
    EXPECT_EQ(hdr.tail, 0);

    hdr.head = 2;
    hdr.tail = 3;
    ASSERT_TRUE(WriteHeader(hFile, hdr));

    QueueHeader hdr2;
    ASSERT_TRUE(ReadHeader(hFile, hdr2));
    EXPECT_EQ(hdr2.head, 2);
    EXPECT_EQ(hdr2.tail, 3);

    CloseHandle(hFile);
    DeleteFileA("test_header.bin");
}

TEST(QueueFileTest, SlotReadWrite)
{
    HANDLE hFile = CreateTestFile("test_slot.bin", 4);
    ASSERT_NE(hFile, INVALID_HANDLE_VALUE);

    MsgRecord rec;
    ZeroMemory(&rec, sizeof(MsgRecord));
    strncpy_s(rec.text, MAX_MSG_LEN, "Hello", _TRUNCATE);
    rec.used = 1;

    ASSERT_TRUE(WriteSlot(hFile, 2, rec));

    MsgRecord rec2;
    ASSERT_TRUE(ReadSlot(hFile, 2, rec2));
    EXPECT_STREQ(rec2.text, "Hello");
    EXPECT_EQ(rec2.used, 1);

    CloseHandle(hFile);
    DeleteFileA("test_slot.bin");
}

TEST(QueueLogicTest, TailWraparound)
{
    const int CAP = 3;
    HANDLE hFile = CreateTestFile("test_wrap.bin", CAP);
    ASSERT_NE(hFile, INVALID_HANDLE_VALUE);

    QueueHeader hdr;
    ReadHeader(hFile, hdr);

    for (int i = 0; i < CAP; i++)
    {
        MsgRecord rec;
        ZeroMemory(&rec, sizeof(MsgRecord));
        sprintf_s(rec.text, MAX_MSG_LEN, "msg%d", i);
        rec.used = 1;
        WriteSlot(hFile, hdr.tail, rec);
        hdr.tail = (hdr.tail + 1) % hdr.capacity;
    }
    WriteHeader(hFile, hdr);

    EXPECT_EQ(hdr.tail, 0);

    CloseHandle(hFile);
    DeleteFileA("test_wrap.bin");
}

TEST(QueueLogicTest, FifoOrder)
{
    const int CAP = 5;
    HANDLE hFile = CreateTestFile("test_fifo.bin", CAP);
    ASSERT_NE(hFile, INVALID_HANDLE_VALUE);

    QueueHeader hdr;
    ReadHeader(hFile, hdr);

    const char* msgs[] = { "aaa", "bbb", "ccc" };
    for (int i = 0; i < 3; i++)
    {
        MsgRecord rec;
        ZeroMemory(&rec, sizeof(MsgRecord));
        strncpy_s(rec.text, MAX_MSG_LEN, msgs[i], _TRUNCATE);
        rec.used = 1;
        WriteSlot(hFile, hdr.tail, rec);
        hdr.tail = (hdr.tail + 1) % hdr.capacity;
    }
    WriteHeader(hFile, hdr);

    ReadHeader(hFile, hdr);
    for (int i = 0; i < 3; i++)
    {
        MsgRecord rec;
        ReadSlot(hFile, hdr.head, rec);
        EXPECT_STREQ(rec.text, msgs[i]);
        hdr.head = (hdr.head + 1) % hdr.capacity;
    }
    EXPECT_EQ(hdr.head, hdr.tail);

    CloseHandle(hFile);
    DeleteFileA("test_fifo.bin");
}

TEST(QueueFileTest, OpenNonExistent)
{
    HANDLE hFile = OpenBinFile("no_such_file_xyz.bin");
    EXPECT_EQ(hFile, INVALID_HANDLE_VALUE);
}

TEST(QueueFileTest, MessageTruncated)
{
    HANDLE hFile = CreateTestFile("test_trunc.bin", 2);
    ASSERT_NE(hFile, INVALID_HANDLE_VALUE);

    MsgRecord rec;
    ZeroMemory(&rec, sizeof(MsgRecord));
    strncpy_s(rec.text, MAX_MSG_LEN, "ABCDEFGHIJKLMNOPQRSTUVWXYZ123456789", _TRUNCATE);
    rec.used = 1;
    WriteSlot(hFile, 0, rec);

    MsgRecord rec2;
    ReadSlot(hFile, 0, rec2);
    EXPECT_LT(static_cast<int>(strlen(rec2.text)), MAX_MSG_LEN);

    CloseHandle(hFile);
    DeleteFileA("test_trunc.bin");
}
