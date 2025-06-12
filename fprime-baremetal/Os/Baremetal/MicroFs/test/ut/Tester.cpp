
#include "Tester.hpp"
#include <stdio.h>
#include <Fw/Test/UnitTest.hpp>
#include <Fw/Types/Assert.hpp>
#include "STest/Random/Random.hpp"

namespace Os {

// ----------------------------------------------------------------------
// Construction and destruction
// ----------------------------------------------------------------------

Tester ::Tester() : alloc() {
}

Tester ::~Tester() {}

Tester ::FileModel ::FileModel() : mode(DOESNT_EXIST), created(false), size(-1) {}

void Tester ::FileModel ::clear() {
    this->curPtr = 0;
    this->size = -1;
    this->created = false;
    memset(this->buffOut, 0xA5, FILE_SIZE);
}

// ----------------------------------------------------------------------
// Tests
// ----------------------------------------------------------------------

void Tester ::NewTest() {
    const U16 NumberBins = 10;
    const U16 NumberFiles = 10;
    const U16 RandomIterations = 500;

    // Instantiate the rules
    InitFileSystem initFileSystem(NumberBins, FILE_SIZE, NumberFiles);
    OpenRandomFile openFile;
    CloseRandomFile closeFile;
    WriteRandomFile writeFile;
    Cleanup cleanup;

    // Apply the rules

    initFileSystem.apply(*this);

    // Run the Rules randomly
    STest::Rule<Tester>* rules[] = {&openFile, &closeFile, &writeFile};
    STest::RandomScenario<Tester> randomScenario("RandomScenario", rules, sizeof(rules) / sizeof(STest::Rule<Tester>*));
    STest::BoundedScenario<Tester> boundedScenario("BoundedScenario", randomScenario, RandomIterations);
    const U32 numSteps = boundedScenario.run(*this);
    ASSERT_EQ(RandomIterations, numSteps);

    cleanup.apply(*this);
}

// ----------------------------------------------------------------------
// CopyTest
// ----------------------------------------------------------------------
void Tester ::CopyTest() {
    const U16 NumberBins = 1;
    const U16 NumberFiles = 2;

    const char* File1 = "/bin0/file0";
    const char* File2 = "/bin0/file1";

    clearFileBuffer();

    // Instantiate the Rules
    InitFileSystem initFileSystem(NumberBins, FILE_SIZE, NumberFiles);
    OpenFile openFile1(File1);
    OpenFile openFile2(File2);
    OpenRead openRead1(File1);
    OpenRead openRead2(File2);
    WriteData writeData(File1);
    CheckFileSize checkFileSize1(File1);
    CheckFileSize checkFileSize2(File2);
    CalcCRC32 calcCRC32(File1);
    ReadData readData1(File1);
    ReadData readData2(File2);
    CloseFile closeFile(File1);
    CopyFile copyFile(File1, File2);
    ResetFile resetFile1(File1);
    ResetFile resetFile2(File2);

    Cleanup cleanup;

    // Run the Rules
    initFileSystem.apply(*this);
    openFile1.apply(*this);
    writeData.apply(*this);
    closeFile.apply(*this);
    copyFile.apply(*this);
    checkFileSize1.apply(*this);
    checkFileSize2.apply(*this);
    openRead1.apply(*this);
    readData1.apply(*this);
    openRead2.apply(*this);
    readData2.apply(*this);
    cleanup.apply(*this);
}
// ----------------------------------------------------------------------
// AppendTest
// ----------------------------------------------------------------------
void Tester ::SimFileTest() {
    SimFileSystem fs(3, 4, 100);  // Initialize the file system with 3 bins and 4 files per bin

    // Get the states of all files
    fs.openFile();

    std::unordered_map<std::string, SimFileSystem::FileState> fileStates = fs.getAllFileStates();
    // Check the state of each file
    for (const auto& fileStatePair : fileStates) {
        std::string filePath = fileStatePair.first;
        SimFileSystem::FileState state = fileStatePair.second;

        if (state == SimFileSystem::FileState::DOES_NOT_EXIST) {
            std::cout << "File " << filePath << " does not exist.\n";
        } else if (state == SimFileSystem::FileState::CLOSED) {
            std::cout << "File " << filePath << " is closed.\n";
        } else if (state == SimFileSystem::FileState::OPENED) {
            std::cout << "File " << filePath << " is open.\n";
        }
    }
}

void Tester ::AppendTest() {
    const U16 NumberBins = 1;
    const U16 NumberFiles = 2;

    const char* File1 = "/bin0/file0";
    const char* File2 = "/bin0/file1";

    clearFileBuffer();

    // Instantiate the Rules
    InitFileSystem initFileSystem(NumberBins, FILE_SIZE, NumberFiles);
    OpenFile openFile1(File1);
    OpenFile openFile2(File2);
    OpenRead openRead1(File1);
    OpenRead openRead2(File2);
    WriteData writeData1(File1);
    WriteData writeData2(File2);
    CheckFileSize checkFileSize1(File1);
    CheckFileSize checkFileSize2(File2);
    CalcCRC32 calcCRC32(File1);
    ReadData readData1(File1);
    ReadData readData2(File2);
    CloseFile closeFile1(File1);
    CloseFile closeFile2(File2);
    AppendFile appendFile(File1, File2);
    ResetFile resetFile1(File1);
    ResetFile resetFile2(File2);

    Cleanup cleanup;

    // Run the Rules
    initFileSystem.apply(*this);
    openFile1.apply(*this);
    writeData1.apply(*this);
    closeFile1.apply(*this);
    openFile2.apply(*this);
    writeData2.apply(*this);
    closeFile2.apply(*this);
    appendFile.apply(*this);
    checkFileSize1.apply(*this);
    checkFileSize2.apply(*this);
    openRead1.apply(*this);
    readData1.apply(*this);
    openRead2.apply(*this);
    readData2.apply(*this);
    cleanup.apply(*this);
}

// ----------------------------------------------------------------------
// OffNominalTests
// ----------------------------------------------------------------------
void Tester ::OffNominalTests() {
    const U16 NumberBins = 1;
    const U16 NumberFiles = 2;

    // Bad bin number
    const char* File1 = "/bin10/file0";

    // Bad file number
    const char* File2 = "/bin0/file10";

    // Already opened
    const char* File3 = "/bin0/file0";

    clearFileBuffer();

    // Instantiate the Rules
    InitFileSystem initFileSystem(NumberBins, FILE_SIZE, NumberFiles);
    Cleanup cleanup;
    OpenFileNotExist openFileNotExist1(File1);
    OpenFileNotExist openFileNotExist2(File2);
    OpenFile openFile1(File3);
    SeekNotOpen seekNotOpen(File3);
    CloseFile closeFile1(File3);

    SeekBadSize seekBadSize(File3, FILE_SIZE + 1);
    SeekRelative seekRelative(File3, 0);
    SeekRelative seekRelative1(File3, FILE_SIZE / 2);
    SeekRelative seekRelative2(File3, FILE_SIZE / 2 - 1);
    SeekRelative seekRelative3(File3, 1);
    ReadNotOpen readNotOpen(File3);
    WriteNotOpen writeNotOpen(File3);
    FlushFile flushFile(File3);
    CopyFile copyFile(File1, File2);
    AppendFile appendFile(File1, File2);
    ReadDirInvalid readDirInvalid("/bin2");
    ReadDirInvalid readDirInvalid2(nullptr);
    ReadDirInvalid readDirInvalid3(" ");

    RemoveInvalid removeInvalid(nullptr);
    RemoveInvalid removeInvalid1("/bin0/file10");

    GetFileSizeInvalid getFileSizeInvalid("/bin0/file10");

    // Run the Rules

    initFileSystem.apply(*this);

    // Bad bin number
    openFileNotExist1.apply(*this);

    // Bad file number
    openFileNotExist2.apply(*this);

    openFile1.apply(*this);

    closeFile1.apply(*this);
    seekNotOpen.apply(*this);

    openFile1.apply(*this);
    seekBadSize.apply(*this);

    seekRelative.apply(*this);
    seekRelative1.apply(*this);
    seekRelative2.apply(*this);
    seekRelative3.apply(*this);
    flushFile.apply(*this);

    closeFile1.apply(*this);
    readNotOpen.apply(*this);
    writeNotOpen.apply(*this);
    flushFile.apply(*this);

    readDirInvalid.apply(*this);
    readDirInvalid2.apply(*this);
    readDirInvalid3.apply(*this);

    removeInvalid.apply(*this);
    removeInvalid1.apply(*this);

    getFileSizeInvalid.apply(*this);

    cleanup.apply(*this);
}

// ----------------------------------------------------------------------
// CrcTest
// ----------------------------------------------------------------------
void Tester ::CrcTest() {
    const U16 NumberBins = 1;
    const U16 NumberFiles = 2;

    const char* File1 = "/bin0/file0";

    clearFileBuffer();

    // Instantiate the Rules
    InitFileSystem initFileSystem(NumberBins, FILE_SIZE, NumberFiles);
    OpenFile openFile(File1);
    OpenRead openRead(File1);
    WriteData writeData(File1);
    CheckFileSize checkFileSize(File1);
    CalcCRC32 calcCRC32(File1);
    ResetFile resetFile(File1);
    ReadData readData(File1);
    CloseFile closeFile(File1);
    Cleanup cleanup;

    // Run the Rules
    initFileSystem.apply(*this);
    openFile.apply(*this);
    writeData.apply(*this);
    checkFileSize.apply(*this);
    closeFile.apply(*this);
    calcCRC32.apply(*this);
    openRead.apply(*this);
    checkFileSize.apply(*this);
    resetFile.apply(*this);
    readData.apply(*this);
    readData.apply(*this);
    readData.apply(*this);
    readData.apply(*this);
    readData.apply(*this);
    readData.apply(*this);
    readData.apply(*this);

    closeFile.apply(*this);
    calcCRC32.apply(*this);

    cleanup.apply(*this);
}

// ----------------------------------------------------------------------
// SeekTest
// ----------------------------------------------------------------------
void Tester ::SeekTest() {
    const U16 NumberBins = 1;
    const U16 NumberFiles = 2;

    const char* File1 = "/bin0/file0";

    clearFileBuffer();

    // Instantiate the Rules
    InitFileSystem initFileSystem(NumberBins, FILE_SIZE, NumberFiles);
    OpenFile openFile(File1);
    ReadData readData(File1);
    WriteData writeData(File1);
    SeekNFile seekNFile0(File1, 0);
    SeekNFile seekNFile25(File1, 25);
    SeekNFile seekNFile50(File1, 50);
    SeekNFile seekNFile99(File1, 99);
    ResetFile resetFile(File1);
    CheckFileSize CheckFileSize(File1);

    Cleanup cleanup;

    // Run the Rules
    initFileSystem.apply(*this);
    openFile.apply(*this);
    writeData.apply(*this);
    writeData.apply(*this);
    writeData.apply(*this);
    writeData.apply(*this);
    writeData.apply(*this);
    writeData.apply(*this);
    writeData.apply(*this);
    writeData.apply(*this);
    seekNFile50.apply(*this);
    writeData.apply(*this);
    seekNFile99.apply(*this);

    cleanup.apply(*this);
}

// ----------------------------------------------------------------------
// DirectoryTest
// ----------------------------------------------------------------------
void Tester ::DirectoryTest() {
    const U16 NumberBins = MAX_BINS;
    const U16 NumberFiles = MAX_FILES_PER_BIN;

    clearFileBuffer();

    // Instantiate the Rules
    InitFileSystem initFileSystem(NumberBins, FILE_SIZE, NumberFiles);
    Cleanup cleanup;

    Directory* directories[MAX_BINS];
    Directory offNominalDir1("/bin10", true);
    Directory offNominalDir2("/bit0", true);
    Directory offNominalDir3("/bin", true);
    Directory offNominalDir4("bin0", true);
    char dirName[MAX_BINS][20];

    for (U16 bin = 0; bin < MAX_BINS; bin++) {
        snprintf(dirName[bin], 20, "/bin%d", bin);
    }

    // Run the Rules
    initFileSystem.apply(*this);

    for (U16 i = 0; i < MAX_BINS; i++) {
        directories[i] = new Directory(dirName[i], false);
        directories[i]->apply(*this);
    }

    offNominalDir1.apply(*this);
    offNominalDir2.apply(*this);
    offNominalDir3.apply(*this);
    offNominalDir4.apply(*this);

    for (U16 i = 0; i < MAX_BINS; i++) {
        delete directories[i];
    }

    cleanup.apply(*this);
}

// ----------------------------------------------------------------------
// MoveTest
// ----------------------------------------------------------------------
void Tester ::MoveTest() {
    const U16 NumberBins = 1;
    const U16 NumberFiles = 3;

    const char* File1 = "/bin0/file0";
    const char* File2 = "/bin0/file1";
    const char* File3 = "/bin0/file10";
    const char* File4 = "/bin0/file2";

    clearFileBuffer();

    // Instantiate the Rules
    InitFileSystem initFileSystem(NumberBins, FILE_SIZE, NumberFiles);
    OpenFile openFile(File1);
    OpenFile openFile2(File2);
    WriteData writeData(File1);
    CheckFileSize checkFileSize(File1);
    CheckFileSize checkFileSize2(File2);
    CloseFile closeFile(File1);
    CloseFile closeFile2(File2);
    Cleanup cleanup;
    MoveFile moveFile(File1, File2);
    IsFileOpen isFileOpen1(File1);
    IsFileOpen isFileOpen2(File2);
    OpenFileNotExist openFileNotExist1(File1);
    OpenFileNotExist openFileNotExist2(File2);
    MoveInvalid moveInvalid(File1, File3);
    MoveInvalid moveInvalid1(File3, File1);
    MoveInvalid moveInvalid2(File1, nullptr);
    MoveInvalid moveInvalid3(nullptr, File1);
    MoveInvalid moveInvalid4(File4, File1);
    MoveBusy moveBusy(File1, File2);
    MoveBusy moveBusy2(File2, File1);

    // Run the Rules
    initFileSystem.apply(*this);
    openFile.apply(*this);
    openFileNotExist2.apply(*this);
    writeData.apply(*this);
    checkFileSize.apply(*this);
    closeFile.apply(*this);
    moveFile.apply(*this);
    openFileNotExist1.apply(*this);
    checkFileSize2.apply(*this);
    isFileOpen1.apply(*this);
    isFileOpen2.apply(*this);
    openFileNotExist1.apply(*this);

    moveInvalid.apply(*this);
    moveInvalid1.apply(*this);
    moveInvalid2.apply(*this);
    moveInvalid3.apply(*this);
    moveInvalid4.apply(*this);

    openFile.apply(*this);
    moveBusy.apply(*this);

    closeFile.apply(*this);
    openFile2.apply(*this);
    moveBusy2.apply(*this);

    cleanup.apply(*this);
}
// ----------------------------------------------------------------------
// OddTests
// ----------------------------------------------------------------------
void Tester ::OddTests() {
    const U16 NumberBins = 1;
    const U16 NumberFiles = 2;

    const char* File1 = "/bin0/file0";
    const char* File2 = "/bin0/file1";

    clearFileBuffer();

    // Instantiate the Rules
    InitFileSystem initFileSystem(NumberBins, FILE_SIZE, NumberFiles);
    OpenRead openRead(File1);
    OpenFile openFile(File1);
    OpenReadEarly openReadEarly(File1);
    OpenCreate openCreate(File1);
    WriteData writeData(File1);
    CheckFileSize checkFileSize(File1);
    CloseFile closeFile(File1);
    CloseFile closeFile2(File2);
    OpenAppend openAppend(File1);
    RemoveFile removeFile(File1);
    RemoveBusyFile removeBusyFile(File1);
    IsFileOpen isFileOpen(File1);
    OpenFile openFile2(File2);
    MoveFile moveFile(File1, File2);
    CheckFileSize checkFileSize2(File2);
    SeekFile seekFile(File1);

    Cleanup cleanup;

    // Run the Rules
    initFileSystem.apply(*this);
    openReadEarly.apply(*this);
    openFile.apply(*this);
    seekFile.apply(*this);
    isFileOpen.apply(*this);
    writeData.apply(*this);
    removeBusyFile.apply(*this);
    closeFile.apply(*this);
    openAppend.apply(*this);
    writeData.apply(*this);
    checkFileSize.apply(*this);
    closeFile.apply(*this);
    removeFile.apply(*this);

    cleanup.apply(*this);
}

// ----------------------------------------------------------------------
// NukeTest
// ----------------------------------------------------------------------
void Tester ::NukeTest() {
    const U16 NumberBins = 1;
    const U16 NumberFiles = 2;
    const U32 RandomIterations = 4000;

    const char* File1 = "/bin0/file0";
    const char* File2 = "/bin0/file1";
    const char* CrcFile = "/bin0/file0.crc32";

    clearFileBuffer();

    // Instantiate the Rules
    InitFileSystem initFileSystem(NumberBins, FILE_SIZE, NumberFiles);
    OpenFile openFile(File1);
    OpenFileNotExist openFileNotExist(CrcFile);
    CloseFile closeFile(File1);
    WriteData writeData(File1);
    ReadData readData(File1);

    CheckFileSize checkFileSize(File1);

    OpenRead openRead1(File1);
    OpenRead openRead2(File2);
    Cleanup cleanup;
    ResetFile resetFile(File1);
    AppendFile appendFile(File1, File2);

    OpenCreate openCreate(File1);
    OpenAppend openAppend(File1);
    RemoveFile removeFile(File1);
    RemoveBusyFile removeBusyFile(File1);
    IsFileOpen isFileOpen(File1);
    SeekFile seekFile(File1);

    // Run the Rules
    initFileSystem.apply(*this);

    // Run the Rules randomly
    STest::Rule<Tester>* rules[] = {&openFile,       &appendFile,       &openRead1,  &openRead2,
                                    &isFileOpen,     &openCreate,       &openAppend, &removeFile,
                                    &removeBusyFile, &openFileNotExist, &closeFile,  &checkFileSize,
                                    &writeData,      &readData,         &resetFile,  &seekFile};
    STest::RandomScenario<Tester> randomScenario("RandomScenario", rules, sizeof(rules) / sizeof(STest::Rule<Tester>*));
    STest::BoundedScenario<Tester> boundedScenario("BoundedScenario", randomScenario, RandomIterations);
    const U32 numSteps = boundedScenario.run(*this);
    ASSERT_EQ(RandomIterations, numSteps);

    cleanup.apply(*this);
}

// ----------------------------------------------------------------------
// FileSizeTest
// ----------------------------------------------------------------------
void Tester ::FileSizeTest() {
    const U16 NumberBins = 1;
    const U16 NumberFiles = 2;

    const char* File1 = "/bin0/file0";

    clearFileBuffer();

    // Instantiate the Rules
    InitFileSystem initFileSystem(NumberBins, FILE_SIZE, NumberFiles);
    OpenFile openFile1(File1);
    CloseFile closeFile(File1);
    WriteData writeData(File1);
    CheckFileSize checkFileSize(File1);
    OpenRead openRead(File1);
    Cleanup cleanup;

    // Run the Rules
    initFileSystem.apply(*this);
    openFile1.apply(*this);
    writeData.apply(*this);
    closeFile.apply(*this);
    checkFileSize.apply(*this);
    openRead.apply(*this);
    closeFile.apply(*this);
    checkFileSize.apply(*this);
    openRead.apply(*this);
    closeFile.apply(*this);
    checkFileSize.apply(*this);

    cleanup.apply(*this);
}

// ----------------------------------------------------------------------
// ReWriteTest
// ----------------------------------------------------------------------
void Tester ::ReWriteTest() {
    const U16 NumberBins = 1;
    const U16 NumberFiles = 2;

    const char* File1 = "/bin0/file0";

    clearFileBuffer();

    // Instantiate the Rules
    InitFileSystem initFileSystem(NumberBins, FILE_SIZE, NumberFiles);
    OpenFile openFile1(File1);
    CloseFile closeFile1(File1);
    Listings listings(NumberBins, NumberFiles);
    WriteData writeData(File1);
    OpenRead openRead1(File1);
    ResetFile resetFile1(File1);
    CheckFileSize checkFileSize(File1);

    Cleanup cleanup;

    // Run the Rules

    // Initialize
    initFileSystem.apply(*this);

    // Part 1:  Open a new file and write max bytes
    printf("Part 1\n");
    openFile1.apply(*this);
    writeData.apply(*this);
    checkFileSize.apply(*this);
    closeFile1.apply(*this);

    // Part 2: Open the file again and write max bytes,
    // check that the size does not exceed tha max
    printf("Part 2\n");
    openFile1.apply(*this);
    writeData.apply(*this);
    checkFileSize.apply(*this);
    closeFile1.apply(*this);

    // Part 3: Open the file again, write half the bytes,
    // check that the size still equals the max, write the
    // other half, check that the size still equals the max.
    printf("Part 3\n");
    openFile1.apply(*this);
    writeData.apply(*this);
    checkFileSize.apply(*this);
    writeData.apply(*this);
    checkFileSize.apply(*this);
    closeFile1.apply(*this);

    // Part 4: Cleanup and reinitialize
    // Open a new file, check the size is 0, write half and check half
    printf("Part 4\n");
    cleanup.apply(*this);
    initFileSystem.apply(*this);
    openFile1.apply(*this);
    checkFileSize.apply(*this);
    writeData.apply(*this);
    checkFileSize.apply(*this);
    closeFile1.apply(*this);

    // Part 5:  Open the file again.  Check size is 1/2
    // Write a 1/4 and check file is still 1/2
    // Write a 1/4 and check file is still 1/2
    // Write a 1/4 and check file is 3/4
    // Write a 1/4 again and check file is full
    printf("Part 5\n");
    openFile1.apply(*this);
    checkFileSize.apply(*this);
    writeData.apply(*this);
    checkFileSize.apply(*this);
    writeData.apply(*this);
    checkFileSize.apply(*this);
    writeData.apply(*this);
    checkFileSize.apply(*this);
    writeData.apply(*this);
    checkFileSize.apply(*this);
    closeFile1.apply(*this);

    // Part

    cleanup.apply(*this);
}

// ----------------------------------------------------------------------
// OpenStressTest
// ----------------------------------------------------------------------
void Tester ::OpenStressTest() {
    const U16 NumberBins = MAX_BINS;
    const U16 NumberFiles = MAX_FILES_PER_BIN;

    const U16 TotalFiles = NumberBins * NumberFiles;
    clearFileBuffer();

    // Instantiate the Rules
    OpenFile* openFile[TotalFiles];
    CloseFile* closeFile[TotalFiles];
    InitFileSystem initFileSystem(NumberBins, FILE_SIZE, NumberFiles);
    Cleanup cleanup;

    char fileName[TotalFiles][20];

    getFileNames(fileName, MAX_BINS, MAX_FILES_PER_BIN);

    // Run the Rules
    initFileSystem.apply(*this);

    for (U16 i = 0; i < TotalFiles; i++) {
        openFile[i] = new OpenFile(fileName[i]);
        openFile[i]->apply(*this);
    }

    for (U16 i = 0; i < TotalFiles; i++) {
        closeFile[i] = new CloseFile(fileName[i]);
        closeFile[i]->apply(*this);
    }

    for (U16 i = 0; i < TotalFiles; i++) {
        delete openFile[i];
    }

    for (U16 i = 0; i < TotalFiles; i++) {
        delete closeFile[i];
    }

    cleanup.apply(*this);
}

// ----------------------------------------------------------------------
// InitTest
// ----------------------------------------------------------------------
void Tester ::InitTest() {
    clearFileBuffer();

    // Instantiate the Rules
    const U16 NumberBins = MAX_BINS;
    const U16 NumberFiles = MAX_FILES_PER_BIN;

    InitFileSystem initFileSystem(NumberBins, FILE_SIZE, NumberFiles);
    Cleanup cleanup;

    // Run the Rules
    initFileSystem.apply(*this);
    cleanup.apply(*this);
}

// ----------------------------------------------------------------------
// OpenWriteReadTest
// ----------------------------------------------------------------------
void Tester ::OpenWriteReadTest() {
    clearFileBuffer();

    // Instantiate the Rules
    const U16 NumberBins = MAX_BINS;
    const U16 NumberFiles = MAX_FILES_PER_BIN;
    const char* FileName = "/bin9/file9";

    InitFileSystem initFileSystem(NumberBins, FILE_SIZE, NumberFiles);
    OpenFile openFile(FileName);
    ResetFile resetFile(FileName);
    CloseFile closeFile(FileName);
    OpenRead openRead(FileName);
    Cleanup cleanup;
    WriteData writeData(FileName);
    ReadData readData(FileName);

    // Run the Rules
    initFileSystem.apply(*this);
    openFile.apply(*this);
    writeData.apply(*this);
    closeFile.apply(*this);
    openRead.apply(*this);
    resetFile.apply(*this);
    readData.apply(*this);

    cleanup.apply(*this);
}

// ----------------------------------------------------------------------
// OpenWriteTwiceReadOnceTest
// ----------------------------------------------------------------------
void Tester ::OpenWriteTwiceReadOnceTest() {
    clearFileBuffer();

    // Instantiate the Rules
    const U16 NumberBins = MAX_BINS;
    const U16 NumberFiles = MAX_FILES_PER_BIN;
    const char* FileName = "/bin0/file0";

    InitFileSystem initFileSystem(NumberBins, FILE_SIZE, NumberFiles);
    OpenFile openFile(FileName);
    ResetFile resetFile(FileName);
    Cleanup cleanup;
    WriteData writeData(FileName);
    ReadData readData(FileName);
    CloseFile closeFile(FileName);
    OpenRead openRead(FileName);

    // Run the Rules
    initFileSystem.apply(*this);
    openFile.apply(*this);
    writeData.apply(*this);
    writeData.apply(*this);
    closeFile.apply(*this);
    openRead.apply(*this);
    resetFile.apply(*this);
    readData.apply(*this);

    cleanup.apply(*this);
}

// ----------------------------------------------------------------------
// OpenWriteOnceReadTwiceTest
// ----------------------------------------------------------------------
void Tester ::OpenWriteOnceReadTwiceTest() {
    clearFileBuffer();

    // Instantiate the Rules
    const U16 NumberBins = MAX_BINS;
    const U16 NumberFiles = MAX_FILES_PER_BIN;
    const char* FileName = "/bin0/file0";

    InitFileSystem initFileSystem(NumberBins, FILE_SIZE, NumberFiles);
    OpenFile openFile(FileName);
    ResetFile resetFile(FileName);
    CloseFile closeFile(FileName);
    Cleanup cleanup;
    OpenRead openRead(FileName);
    WriteData writeData(FileName);
    ReadData readData(FileName);

    // Run the Rules
    initFileSystem.apply(*this);
    openFile.apply(*this);
    writeData.apply(*this);
    closeFile.apply(*this);
    openRead.apply(*this);
    resetFile.apply(*this);
    readData.apply(*this);
    readData.apply(*this);

    cleanup.apply(*this);
}

// ----------------------------------------------------------------------
// ListTest
// ----------------------------------------------------------------------
void Tester ::ListTest() {
    clearFileBuffer();

    // Instantiate the Rules
    const U16 NumberBins = MAX_BINS;
    const U16 NumberFiles = MAX_FILES_PER_BIN;
    const U16 TotalFiles = MAX_BINS * MAX_FILES_PER_BIN;

    OpenFile* openFile[TotalFiles];

    char fileName[TotalFiles][20];
    getFileNames(fileName, MAX_BINS, MAX_FILES_PER_BIN);

    // Instantiate the Rules
    InitFileSystem initFileSystem(NumberBins, FILE_SIZE, NumberFiles);
    Cleanup cleanup;
    Listings listings(NumberBins, NumberFiles);

    // Run the Rules
    initFileSystem.apply(*this);

    for (U16 i = 0; i < TotalFiles; i++) {
        openFile[i] = new OpenFile(fileName[i]);
        openFile[i]->apply(*this);
    }

    listings.apply(*this);

    for (U16 i = 0; i < TotalFiles; i++) {
        delete openFile[i];
    }

    cleanup.apply(*this);
}

// ----------------------------------------------------------------------
// OpenWriteReadTest
// ----------------------------------------------------------------------
void Tester ::OpenFreeSpaceTest() {
    clearFileBuffer();

    // Instantiate the Rules
    U32 numBins = 1;
    U32 numFiles = 1;
    const char* FileName = "/bin0/file0";
    InitFileSystem initFileSystem(numBins, FILE_SIZE, numFiles);

    FreeSpace freeSpace;
    OpenFile openFile(FileName);
    // ResetFile resetFile;
    Cleanup cleanup;
    WriteData writeData(FileName);

    // Run the Rules
    initFileSystem.apply(*this);
    this->m_expFreeBytes = 100;
    this->m_expTotalBytes = 100;
    freeSpace.apply(*this);
    openFile.apply(*this);
    writeData.apply(*this);
    this->m_expFreeBytes = 0;
    this->m_expTotalBytes = 100;
    freeSpace.apply(*this);

    cleanup.apply(*this);
}

// Helper functions
void Tester::clearFileBuffer() {
    for (U32 i = 0; i < MAX_TOTAL_FILES; i++) {
        this->fileModels[i].clear();
    }
}

FwIndexType Tester::getIndex(const char* fileName) const {
    const char* filePathSpec = "/bin%" MICROFS_INDEX_SCN_FORMAT "/file%" MICROFS_INDEX_SCN_FORMAT;

    FwIndexType binIndex = 0;
    FwIndexType fileIndex = 0;

    int stat = sscanf(fileName, filePathSpec, &binIndex, &fileIndex);
    if (stat != 2) {
        return -1;

    } else {
        FW_ASSERT(binIndex < MAX_BINS, binIndex);
        FW_ASSERT(fileIndex < MAX_FILES_PER_BIN, fileIndex);
        return binIndex * MAX_BINS + fileIndex;
    }
}

Tester::FileModel* Tester::getFileModel(const char* filename) {
    I16 fileIndex = this->getIndex(filename);

    FW_ASSERT(fileIndex < MAX_TOTAL_FILES && fileIndex >= 0, fileIndex);

    return &(this->fileModels[fileIndex]);
}

void Tester::getFileNames(char fileNames[][20], U16 numBins, U16 numFiles) {
    int count = 0;
    for (int i = 0; i < numBins; i++) {
        for (int j = 0; j < numFiles; j++) {
            sprintf(fileNames[count], "/bin%d/file%d", i, j);  // format the string
            count++;
        }
    }
}

}  // end namespace Os
