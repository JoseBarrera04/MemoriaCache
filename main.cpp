/**
 * @authors Maria Lucia Castillo Garcia y Jose Barrera Ramos
 *
 * Especificaciones Memoria Cache:
 * - Asociativa 2 Vias
 * - Politica Reemplazo LRU
 * - Politica Actualizacion WRITE BACK
 */

// Librerias
#include <iostream>
#include <vector>
#include <iomanip>
#include <string>
#include <random>
#include <cstdio>
#include <cstring>

using namespace std;

// Definiciones
#define BLOCK_SIZE 16
#define MEMORY_SIZE 2048
#define CACHE_SETS 16
#define AXS 2

// Structs
struct CacheLine {
    unsigned int tag;
    bool valid;
    bool dirty;
    int lru;
    vector<unsigned char> data;

    CacheLine() : tag(0), valid(false), dirty(false), lru(0), data(BLOCK_SIZE, 0) {}
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
void updateMemoryFile();

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
 * Actualizar el archivo memory.txt con el contenido de la memoria
 */
void updateMemoryFile() {
    FILE *file = fopen("C:/Users/JoseB/Desktop/MemoriaCache/data/memory.txt", "w");
    if (file == NULL) {
        cerr << "Error al abrir memory.txt para escritura." << endl;
        return;
    }

    for (int k = 0; k < MEMORY_SIZE; k++) {
        fprintf(file, "%d\n", memory[k]);
    }

    fclose(file);
    cout << "memory.txt actualizado tras write-back." << endl;
}

/**
 * Inicializar los datos de la cache a su manera predeterminada
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
 * Funcion para leer los datos de la memoria cache
 * @param filename
 */
void loadData(const string& filename) {
    FILE *file = fopen("C:/Users/JoseB/Desktop/MemoriaCache/data/memory.txt", "r");
    int valueAux;
    if (file == NULL || fscanf(file, "%d", &valueAux) != 1) {
        srand(time(NULL));
        for (int i = 0; i < MEMORY_SIZE; i++) {
            int tipo = rand() % 2; // 0: número, 1: letra
            if (tipo == 0) {
                memory[i] = '0' + (rand() % 256); // '0' a '256'
            } else {
                memory[i] = 'A' + (rand() % 6);  // 'A' a 'F'
            }
        }
        cout << "Archivo de memoria no encontrado o vacío. Memoria inicializada con datos aleatorios." << endl;
        updateMemoryFile();
        return;
    }

    int i = 0;
    int value;
    while (fscanf(file, "%d", &value) == 1 && i < MEMORY_SIZE) {
        memory[i++] = static_cast<unsigned char>(value);
    }
    fclose(file);
}

/**
 * Funcion para leer datos desde la cache
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
        cout << "Write-back: Actualizando memoria en dirección " << hex << blockStart << dec << endl;
        for (int i = 0; i < BLOCK_SIZE; i++) {
            memory[blockStart + i] = set->lines[lruIndex].data[i];
        }
        updateMemoryFile();
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
 * Funcion para sobreescribir un dato en la memoria cache
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
        cout << "Write-back: Actualizando memoria en dirección " << hex << blockStart << dec << endl;
        for (int i = 0; i < BLOCK_SIZE; i++) {
            memory[blockStart + i] = set->lines[lruIndex].data[i];
        }
        updateMemoryFile();
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

    cout << "Escritura exitosa en el nuevo bloque cargado en cache. Marcando como dirty." << endl;
}

/**
 * Funcion para leer instrucciones de la cache
 * @param filename
 */
void loadActions(const string& filename) {
    FILE *file = fopen("data/actions.txt", "r");
    if (file == NULL || fgetc(file) == EOF) {
        if (file != NULL) fclose(file);
        cout << "Archivo de acciones vacío o no encontrado. Generando instrucciones aleatorias." << endl;
        srand(time(NULL));
        int numActions = 10 + rand() % 11; // Entre 10 y 20 acciones
        for (int i = 0; i < numActions; i++) {
            int actionType = rand() % 2; // 0: READ, 1: WRITE
            unsigned int address = rand() % MEMORY_SIZE; // Rango: 0 a 2047
            if (actionType == 0) {
                cout << "Generado: READ " << hex << address << dec << endl;
                readFromCache(address);
            } else {
                unsigned char value = rand() % 256; // Valor entre 0 y 255
                cout << "Generado: WRITE " << hex << address << " " << static_cast<int>(value) << dec << endl;
                writeToCache(address, value);
            }
        }
        return;
    }

    rewind(file);
    char action[10];
    char addressStr[20];
    unsigned int address;
    int value;

    while (fscanf(file, "%9s %19s", action, addressStr) == 2) {
        string addr(addressStr);
        if (addr.size() > 2 && addr[0] == '0' && (addr[1] == 'x' || addr[1] == 'X')) {
            address = stoul(addr, nullptr, 16);
        } else {
            address = stoul(addr, nullptr, 10);
        }

        if (strcmp(action, "READ") == 0) {
            readFromCache(address);
        } else if (strcmp(action, "WRITE") == 0) {
            if (fscanf(file, "%d", &value) != 1) {
                cerr << "Error al leer el valor para WRITE." << endl;
                continue;
            }
            writeToCache(address, static_cast<unsigned char>(value));
        } else {
            cerr << "Accion desconocida: " << action << endl;
        }
    }

    fclose(file);
}

/**
 * Funcion para imprimir los datos de la cache
 */
void printInfoCache() {
    FILE *file = fopen("C:/Users/JoseB/Desktop/MemoriaCache/data/cacheOutput.txt", "w");
    if (file == NULL) {
        cerr << "Error al crear el archivo de salida." << endl;
        return;
    }

    fprintf(file, "Estado de la Cache:\n");
    fprintf(file, "Conjunto | Via | Valid | Dirty | Tag | Rango Direccion | Datos |\n");
    fprintf(file, "---------------------------------------------------------------\n");
    cout << "\nEstado de la Cache:\n";
    cout << "Conjunto | Via | Valid | Dirty | Tag | Rango Direccion | Datos |\n";
    cout << "---------------------------------------------------------------\n";

    for (int i = 0; i < CACHE_SETS; i++) {
        for (int j = 0; j < AXS; j++) {
            CacheLine* line = &cache[i].lines[j];
            unsigned int blockStartAddress = (line->tag * CACHE_SETS + i) * BLOCK_SIZE;
            unsigned int blockEndAddress = blockStartAddress + BLOCK_SIZE - 1;

            fprintf(file, "%d | %d | %d | %d | %5u | %08x-%08x |", i, j, line->valid, line->dirty, line->tag, blockStartAddress, blockEndAddress);
            for (int k = 0; k < BLOCK_SIZE; k++) {
                fprintf(file, " %d", static_cast<int>(line->data[k]));
            }
            fprintf(file, "\n");

            cout << i << " | " << j << " | " << line->valid << " | " << line->dirty << " | "
                 << setw(5) << line->tag << " | " << hex << setw(8) << setfill('0')
                 << blockStartAddress << "-" << setw(8) << blockEndAddress << " | " << dec;
            for (int k = 0; k < BLOCK_SIZE; k++) {
                cout << " " << static_cast<int>(line->data[k]);
            }
            cout << endl;
        }
    }

    fprintf(file, "---------------------------------------------------------------\n");
    cout << "---------------------------------------------------------------\n";
    fclose(file);
}

/**
 * Funcion para mostrar datos de los miss and hits
 */
void printCacheStatistics() {
    FILE *file = fopen("C:/Users/JoseB/Desktop/MemoriaCache/data/misshitsplot.txt", "w");
    if (file == NULL) {
        cerr << "Error al crear el archivo de estadísticas." << endl;
        return;
    }

    int totalRequests = hitCount + missCount;
    double hitPercentage = (totalRequests > 0) ? (static_cast<double>(hitCount) / totalRequests * 100) : 0;
    double missPercentage = (totalRequests > 0) ? (static_cast<double>(missCount) / totalRequests * 100) : 0;

    cout << fixed << setprecision(2);
    cout << "Hit Percentage: " << hitPercentage << "%\n";
    cout << "Miss Percentage: " << missPercentage << "%\n";

    fprintf(file, "Hit Percentage: %.2f%%\n", hitPercentage);
    fprintf(file, "Miss Percentage: %.2f%%\n", missPercentage);

    fclose(file);
}