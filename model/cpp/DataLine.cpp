#include "../h/DataLine.h"

using namespace std;

//Constructor
DataLine::DataLine() {
}

//Funciones


//Getters y Setters
bool DataLine::getDirty() {
    return dirty;
}

bool DataLine::getValid() {
    return valid;
}

unsigned int DataLine::getTag() {
    return tag;
}

unsigned int DataLine::getData() {
    return data;
}

unsigned int DataLine::getLFU() {
    return lfu;
}


void DataLine::setDirty(bool dirty) {
    this->dirty = dirty;
}

void DataLine::setValid(bool valid) {
    this->valid = valid;
}

void DataLine::setTag(unsigned int tag) {
    this->tag = tag;
}

void DataLine::setData(unsigned int data) {
    this->data = data;
}

void DataLine::setLFU(unsigned int lfu) {
    this->lfu = lfu;
}