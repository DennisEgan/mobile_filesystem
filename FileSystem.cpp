#include "FileSystem.h"
#include <iostream>
#include <cstring>
#include <vector>
using namespace std;

// Constructor
FileSystem::FileSystem():myDisk(BLOCK_SIZE, DISK_SIZE){
	// for (int i = 0; i <DISK_SIZE; i++){
	// 	DiskBlockType *myBuffer = new DiskBlockType(BLOCK_SIZE);
	// 	if (i < (DISK_SIZE-1))
	// 		myBuffer->data[0] = (unsigned char)(i+1);
	// 	else
	// 		myBuffer->data[0] = (unsigned char)(-1);
	// 	myDisk.write(i, myBuffer);
	// 	delete myBuffer;
	// }
	freespace = new FCB(0, 0, "freespace");
	freespace->setBlockSize(DISK_SIZE);
}

// Deconstructor
FileSystem::~FileSystem(){
	delete freespace;
	for (int i = 0; i < files.size(); i++){
		delete files[i];
		files.erase(files.begin()+i)
		// files[i] = NULL;
	}
}

// Create file with name fileName
bool FileSystem::create(string fileName){
	FCB* newfile = new FCB(0, -1, fileName);
	files.push_back(newfile);
	// cout << "newfile" << endl;
	// newfile->print();
	// cout << "freespace" << endl;
	// freespace->print();
	// cout << endl;
}

// Open file in specified mode
int FileSystem::open(string filename, string mode){
	cout << "opening " << filename << "..." << endl;
	
	FCB *tmp = NULL;

	int searchVal = search(filename);
	if (searchVal != -1){
		if (mode == "w"){
			tmp = new FCB(*(files[searchVal]));
			cout << "tmp:" << endl;
			tmp->print();
			FOT.push_back(tmp);
		}
	}
	// cout << "TEMPORARY: " << tmp << endl;
	// cout << "REAL: " << files[0] << endl;
}

bool FileSystem::close(int handle){
	cout << "closing " << FOT[handle]->getFileName() << "..." << endl;
	int searchVal = search(FOT[handle]->getFileName());
	delete files[searchVal];
	files[searchVal] = NULL;
	files[searchVal] = new FCB(*FOT[handle]);
	// cout << "file:" << endl;
	// files[searchVal]->print();
	// cout << "FOT[Handle] ADDRESS:" << FOT[handle] << endl;
	// FCB* tmp = FOT[handle];
	// cout << "tmp ADDRESS:" << tmp << endl;
	// delete tmp;
	// tmp = NULL;
	delete FOT[handle];
	FOT.erase(FOT.begin() + handle);
	//cout << "FOT[0]" << (FOT[0]) << endl;
	//FOT[0]->print();
}

int FileSystem::read(int handle, int numchars, char *buffer){
	cout << "Number Of Chars to Read " << numchars << endl;
	int nextBlockToRead = 0;
	int blockToRead = FOT[handle]->getBlockPointer();
	while (numchars > 0){
		DiskBlockType *readBuffer = new DiskBlockType(BLOCK_SIZE);
		myDisk.read(blockToRead, readBuffer);
		nextBlockToRead = readBuffer->data[0];
		//cout << endl << "reading block " << blockToRead << endl;
		for (int i = 1; i < BLOCK_SIZE; i++){
			//cout << "i = " << i << endl;
			// DiskBlockType *testBuffer = new DiskBlockType(BLOCK_SIZE);
			// myDisk.read(blockToRead, testBuffer);
			//printf("Reading Back What I wrote[%d]: next block->%d data:%s\n", blockToRead, testBuffer->data[0],&testBuffer->data[ strlen(testBuffer->data) -4 ]);
			// delete testBuffer;
			// testBuffer = NULL;
			if (numchars == 0)
				break;
			printf("%c", readBuffer->data[i]);
			numchars--;

		}
		if (nextBlockToRead != -1){
			blockToRead = nextBlockToRead;
		}
		delete readBuffer;
		readBuffer = NULL;
	}
	cout << endl;
}
int FileSystem::write(int handle, int numchars, char* buffer){
	cout << "Writing to " << FOT[handle]->getFileName() << "..." << endl;
	cout << "Number of chars to write = " << numchars << endl;
	int sentinel = -1;
	int blockToWrite = 0;
	int nextBlockToWrite = 0;
	int readChar = 0;
	int writeChar = 0;
	bool blockFull = false;
	int offset = 0;
	int charsWritten = 0;
	//first step: check if file is empty
	if (FOT[handle]->getSize() == 0){
		//file was empty
		if (freespace->getBlockSize() > 0){
			FOT[handle]->setBlockPointer(getFreeBlock());
			FOT[handle]->setFileEnd(FOT[handle]->getBlockPointer());
			FOT[handle]->setBlockSize(FOT[handle]->getBlockSize()+1);
			offset = 1;
		}
		else{
			cout << "Error, no blocks available1" << endl;
			cout << "\tChars Written = " << charsWritten << endl;
			return charsWritten; // 0
		}
	}
	//if file is not empty, calculate offset
	if (FOT[handle]->getSize() != 0){
		offset = (FOT[handle]->getSize()%(BLOCK_SIZE-1));
		//if offset is 0, that block is full
		if (offset == 0){
			if (freespace->getBlockSize() > 0){
				offset = 1;
				nextBlockToWrite = getFreeBlock();
				DiskBlockType *myBuffer = new DiskBlockType(BLOCK_SIZE);
				blockToWrite = FOT[handle]->getFileEnd();
				myDisk.read(blockToWrite,myBuffer);
				myBuffer->data[0] = nextBlockToWrite;
				myDisk.write(blockToWrite,myBuffer);
				blockToWrite = nextBlockToWrite;
				FOT[handle]->setFileEnd(blockToWrite);
				FOT[handle]->setBlockSize(FOT[handle]->getBlockSize()+1);
				nextBlockToWrite = -1;
				delete myBuffer;
				myBuffer = NULL;
			}
			else{
				cout << "Error, no blocks available2" << endl;
				cout << "\tChars Written = " << charsWritten << endl;
				return charsWritten;
			}
		}else{
			offset++;
		}
	}
	while(numchars > 0){
		blockToWrite = FOT[handle]->getFileEnd();
		DiskBlockType *writeBuffer = new DiskBlockType(BLOCK_SIZE);
		cout << "Writing to block ===== " << blockToWrite << endl;
		if(offset > 1){
			myDisk.read(blockToWrite,writeBuffer);
		}
		for (int i = offset; i < BLOCK_SIZE; i++){
			if (numchars == 0)
				break;
			if (i == BLOCK_SIZE-1)
				blockFull = true;
			writeBuffer->data[i] = buffer[readChar];
			numchars--;
			readChar++;
			charsWritten++;
			FOT[handle]->setSize(FOT[handle]->getSize()+1);
		}
		// if (blockToWrite == DISK_SIZE-1){
		// 	cout << "LAST BLOCK AVAILABLE" << endl;
		// 	writeBuffer->data[0] = (char)(sentinel);
		// 	myDisk.write(blockToWrite,writeBuffer);
		// 	return charsWritten;
		// }
		if ((blockFull && numchars > 0)){
			if (freespace->getBlockSize() > 0){
				nextBlockToWrite = getFreeBlock();
				DiskBlockType *myBuffer = new DiskBlockType(BLOCK_SIZE);
				blockToWrite = FOT[handle]->getFileEnd();
				writeBuffer->data[0] = nextBlockToWrite;
				myDisk.write(blockToWrite,writeBuffer);
				delete myBuffer;
				myBuffer = NULL;
				blockToWrite = nextBlockToWrite;
				FOT[handle]->setFileEnd(blockToWrite);
				FOT[handle]->setBlockSize(FOT[handle]->getBlockSize()+1);
				offset = 1;
			}
			else{
				writeBuffer->data[0] = (char)(sentinel);
				myDisk.write(blockToWrite,writeBuffer);
				cout << "No blocks available3" << endl;
				cout << "\tChars Written = " << charsWritten << endl;
				return charsWritten;
			}
		}else{
			writeBuffer->data[0] = (char)(sentinel);
			myDisk.write(blockToWrite,writeBuffer);
		}
		blockFull = false;
		delete writeBuffer;
		writeBuffer = NULL;
	}
	cout << "\tChars Written = " << charsWritten << endl;
	return charsWritten;
}

int FileSystem::getFreeBlock(){
	int blockIndex = 0;
	if (freespace->getBlockSize() != 0){
		blockIndex = freespace->getBlockPointer();//save block location
		freespace->setBlockSize(freespace->getBlockSize()-1);//decrement freespace size
		freespace->setBlockPointer(freespace->getBlockPointer()+1);//move freespace pointer forward
	}
	else{
		blockIndex = -1;
	}
	cout << "popping block " << blockIndex << endl;
	cout << "freespace blocks left = " << freespace->getBlockSize() << "/" << DISK_SIZE << endl;
	return blockIndex;
}
int FileSystem::search(string fileName){
	int index = -1;
	for (int i = 0; i < files.size(); i++){
		if (files[i]->getFileName() == fileName){
			index = i;
		}
	}
	return index;
}
int FileSystem::getNumChars(int file){
	files[file]->print();
	cout << "file size = " << files[file]->getSize() << endl;
	return FOT[file]->getSize();
}

bool FileSystem::delete(string fileName)
{
		
}