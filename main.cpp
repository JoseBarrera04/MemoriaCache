/**
 * @authors Maria Lucia Castillo Garcia y Jose Barrera Ramos
 *
 * Especificaciones Memoria Cache:
 * - Asociativa 2 Vias
 * - Politica Reemplazo LFU
 * - Politica Actualizacion WRITE BACK
 */

// Librerias
#include <iostream>
#include <vector>
#include <iomanip>
#include <fstream>
#include <string>

using namespace std;

// Definiciones
#define BLOCK_SIZE 16
#define MEMORY_SIZE 2048
#define CACHE_SETS 32
#define AXS 2

// Structs
struct CacheLine {
    unsigned int tag;
    bool valid;
    bool dirty;
    int lru;
    vector<unsigned char> data;

    CacheLine() : valid(false), dirty(false), lru(0), data(BLOCK_SIZE, 0) {}
};

struct CacheSet {
    vector<CacheLine> lines;

    CacheSet() : lines(AXS) {}
};

// Variables globales
CacheSet cache[CACHE_SETS];
vector<unsigned char> memory(MEMORY_SIZE, 0);
int hitCount = 0;
int missCount = 0;

// Funciones
unsigned int getSetIndex(unsigned int address);
unsigned int getTag(unsigned int address);
unsigned int getOffset(unsigned int address);
void startCache();
void loadData(const string& filename);
void readFromCache(unsigned int address);
void writeToCache(unsigned int address, unsigned char value);
void loadActions(const string& filename);
void printInfoCache();
void printCacheStatistics();

/**
 * Main Principal
 * @return
 */
int main() {
    startCache();
    loadData("data/memory.txt");
    loadActions("data/actions.txt");
    printInfoCache();
    printCacheStatistics();
    return 0;
}

unsigned int getSetIndex(unsigned int address) {
    return (address / BLOCK_SIZE) % CACHE_SETS;
}

unsigned int getTag(unsigned int address) {
    return (address / BLOCK_SIZE) / CACHE_SETS;
}

unsigned int getOffset(unsigned int address) {
    return address % BLOCK_SIZE;
}

/**
 *
 */
void startCache() {
    for (int i = 0; i < CACHE_SETS; i++) {
        cache[i].lines.resize(AXS);
        for (int j = 0; j < AXS; j++) {
            cache[i].lines[j].valid = false;
            cache[i].lines[j].dirty = false;
            cache[i].lines[j].lru = 0;
        }
    }
}

/**
 *
 * @param filename
 */
void loadData(const string& filename) {
    ifstream file("C:/Users/JoseB/Desktop/MemoriaCache/data/memory.txt");
    if (!file.is_open()) {
        cerr << "Error al abrir el archivo de datos de memoria." << endl;
        return;
    }

    int i = 0;
    int value;
    while (file >> value && i < MEMORY_SIZE) {
        memory[i++] = static_cast<unsigned char>(value);
    }
    file.close();
}

/**
 *
 * @param address
 */
void readFromCache(unsigned int address) {
    unsigned int setIndex = getSetIndex(address);
    unsigned int tag = getTag(address);
    unsigned int offset = getOffset(address);

    CacheSet* set = &cache[setIndex];

    for (int i = 0; i < AXS; i++) {
        if (set->lines[i].valid && set->lines[i].tag == tag) {
            unsigned char value = set->lines[i].data[offset];
            cout << "Cache hit! Leyendo dato: " << static_cast<int>(value) << endl;
            hitCount++;
            for (int j = 0; j < AXS; j++) {
                set->lines[j].lru = (j != i) ? set->lines[j].lru + 1 : 0;
            }
            return;
        }
    }

    missCount++;
    cout << "Cache miss! Cargando bloque desde la memoria." << endl;

    int lruIndex = (set->lines[0].lru > set->lines[1].lru) ? 0 : 1;

    if (set->lines[lruIndex].valid && set->lines[lruIndex].dirty) {
        unsigned int blockStart = (set->lines[lruIndex].tag * CACHE_SETS + setIndex) * BLOCK_SIZE;
        for (int i = 0; i < BLOCK_SIZE; i++) {
            memory[blockStart + i] = set->lines[lruIndex].data[i];
        }
    }

    unsigned int blockStart = (address / BLOCK_SIZE) * BLOCK_SIZE;
    for (int i = 0; i < BLOCK_SIZE; i++) {
        set->lines[lruIndex].data[i] = memory[blockStart + i];
    }
    set->lines[lruIndex].tag = tag;
    set->lines[lruIndex].valid = true;
    set->lines[lruIndex].dirty = false;

    for (int j = 0; j < AXS; j++) {
        set->lines[j].lru = (j != lruIndex) ? set->lines[j].lru + 1 : 0;
    }

    unsigned char value = set->lines[lruIndex].data[offset];
    cout << "Dato cargado desde memoria: " << static_cast<int>(value) << endl;
}

/**
 *
 * @param address
 * @param value
 */
void writeToCache(unsigned int address, unsigned char value) {
    unsigned int setIndex = getSetIndex(address);
    unsigned int tag = getTag(address);
    unsigned int offset = getOffset(address);

    CacheSet* set = &cache[setIndex];

    for (int i = 0; i < AXS; i++) {
        if (set->lines[i].valid && set->lines[i].tag == tag) {
            set->lines[i].data[offset] = value;
            set->lines[i].dirty = true;
            cout << "Cache hit! Escritura exitosa en cache y marcando como dirty." << endl;
            hitCount++;
            for (int j = 0; j < AXS; j++) {
                set->lines[j].lru = (j != i) ? set->lines[j].lru + 1 : 0;
            }
            return;
        }
    }

    missCount++;
    cout << "Cache miss! Cargando bloque para escritura." << endl;

    int lruIndex = (set->lines[0].lru > set->lines[1].lru) ? 0 : 1;

    if (set->lines[lruIndex].valid && set->lines[lruIndex].dirty) {
        unsigned int blockStart = (set->lines[lruIndex].tag * CACHE_SETS + setIndex) * BLOCK_SIZE;
        for (int i = 0; i < BLOCK_SIZE; i++) {
            memory[blockStart + i] = set->lines[lruIndex].data[i];
        }
    }

    unsigned int blockStart = (address / BLOCK_SIZE) * BLOCK_SIZE;
    for (int i = 0; i < BLOCK_SIZE; i++) {
        set->lines[lruIndex].data[i] = memory[blockStart + i];
    }
    set->lines[lruIndex].tag = tag;
    set->lines[lruIndex].valid = true;
    set->lines[lruIndex].dirty = true;

    set->lines[lruIndex].data[offset] = value;

    for (int j = 0; j < AXS; j++) {
        set->lines[j].lru = (j != lruIndex) ? set->lines[j].lru + 1 : 0;
    }

    cout << "Escritura exitosa en el nuevo bloque cargado en caché. Marcando como dirty." << endl;
}

/**
 *
 * @param filename
 */
void loadActions(const string& filename) {
    ifstream file("C:/Users/JoseB/Desktop/MemoriaCache/data/actions.txt");
    if (!file.is_open()) {
        cerr << "Error al abrir el archivo de acciones." << endl;
        return;
    }

    string action;
    unsigned int address;
    int value;

    while (file >> action >> hex >> address) {
        if (action == "READ") {
            readFromCache(address);
        } else if (action == "WRITE") {
            file >> dec >> value;
            writeToCache(address, static_cast<unsigned char>(value));
        } else {
            cerr << "Accion desconocida: " << action << endl;
        }
    }
    file.close();
}

/**
 *
 */
void printInfoCache() {
    ofstream outputFile("cache_output.txt");
    if (!outputFile.is_open()) {
        cerr << "Error al crear el archivo de salida." << endl;
        return;
    }

    cout << "\nEstado de la Cache:\n";
    cout << "Conjunto | Via | Valid | Dirty | Tag | Rango Direccion | Datos\n";
    cout << "---------------------------------------------------------------\n";

    for (int i = 0; i < CACHE_SETS; i++) {
        for (int j = 0; j < AXS; j++) {
            CacheLine* line = &cache[i].lines[j];
            unsigned int blockStartAddress = (line->tag * CACHE_SETS + i) * BLOCK_SIZE;
            unsigned int blockEndAddress = blockStartAddress + BLOCK_SIZE - 1;

            cout << setw(8) << i << " | " << j << " | " << line->valid << " | " << line->dirty << " | "
                 << setw(5) << line->tag << " | " << hex << setw(8) << setfill('0')
                 << blockStartAddress << "-" << setw(8) << blockEndAddress << " | " << dec;

            for (int k = 0; k < BLOCK_SIZE; k++) {
                cout << " " << setw(3) << static_cast<int>(line->data[k]);
            }
            cout << endl;
        }
    }

    for (int k = 0; k < MEMORY_SIZE; k++) {
        outputFile << static_cast<int>(memory[k]) << endl;
    }
    cout << "---------------------------------------------------------------\n";
    outputFile.close();
}

/**
 *
 */
void printCacheStatistics() {
    ofstream file("miss_hits_plot.txt");
    if (!file.is_open()) {
        cerr << "Error al crear el archivo de estadísticas." << endl;
        return;
    }

    int totalRequests = hitCount + missCount;
    double hitPercentage = (totalRequests > 0) ? (static_cast<double>(hitCount) / totalRequests * 100) : 0;
    double missPercentage = (totalRequests > 0) ? (static_cast<double>(missCount) / totalRequests * 100) : 0;

    cout << fixed << setprecision(2);
    cout << "Hit Percentage: " << hitPercentage << "%\n";
    cout << "Miss Percentage: " << missPercentage << "%\n";
    file << "Hit Percentage: " << hitPercentage << "%\n";
    file << "Miss Percentage: " << missPercentage << "%\n";
    file.close();
}