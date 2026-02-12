#include <iostream>
#include <fstream>
#include <cstring>
#include <iomanip>
#include <vector>
#include <algorithm>
#include <cstdlib>

using namespace std;

/* ================== CONSTANTES ================== */
const int ACCION_LEN = 12;
const int BOLSA_LEN = 14;
const int CANT_PLAZOS = 4;

/* ================== ESTRUCTURAS ================== */
// IMPORTANTE: SIN +1 en los arrays para el binario
#pragma pack(push, 1)
struct Operacion {
    char accion[ACCION_LEN];      // 12 bytes, SIN null terminator
    int plazo;
    char bolsa[BOLSA_LEN];        // 14 bytes, SIN null terminator
    float precioUnitario;
    int cantidad;
};
#pragma pack(pop)

// Para el listado (YA con null terminator)
struct OperacionInfo {
    char accion[ACCION_LEN + 1];
    int cantidad;
    bool esCompra;
    int ordenOriginal;
};

struct BolsaResumen {
    char nombre[BOLSA_LEN + 1];
    float montoTotal;
    float resultado;
    vector<OperacionInfo> operaciones;
};

struct PlazoResumen {
    int codigo;
    const char* nombre;
    int compras;
    int ventas;
    vector<BolsaResumen> bolsas;
};

const char* NOMBRE_PLAZOS[] = {"CI", "24Hs", "48Hs", "72Hs"};

/* ================== FUNCIONES AUXILIARES ================== */

void normalizarString(char* destino, const char* origen, int len) {
    strncpy(destino, origen, len);
    destino[len] = '\0';
}

int buscarBolsa(vector<BolsaResumen>& bolsas, const char* nombre) {
    for (size_t i = 0; i < bolsas.size(); i++) {
        if (strcmp(bolsas[i].nombre, nombre) == 0)
            return i;
    }
    return -1;
}

/* ================== LECTURA Y PROCESAMIENTO EN UN SOLO PASO ================== */

vector<PlazoResumen> leerYProcesar(const char* nombre) {
    vector<PlazoResumen> plazos;
    
    // Inicializar los 4 plazos
    for (int i = 0; i < CANT_PLAZOS; i++) {
        PlazoResumen p;
        p.codigo = i;
        p.nombre = NOMBRE_PLAZOS[i];
        p.compras = 0;
        p.ventas = 0;
        plazos.push_back(p);
    }
    
    ifstream arch(nombre, ios::binary);
    if (!arch) {
        cerr << "Error: No se pudo abrir " << nombre << endl;
        return plazos;
    }
    
    // Calcular cantidad de registros
    arch.seekg(0, ios::end);
    int bytes = arch.tellg();
    arch.seekg(0, ios::beg);
    int totalRegistros = bytes / sizeof(Operacion);
    
    // Mostrar encabezado
    cout << left;
    cout << setw(ACCION_LEN + 2) << "Accion"
         << setw(8) << "Plazo"
         << setw(BOLSA_LEN + 2) << "Bolsa"
         << setw(12) << "Pre.Uni."
         << setw(10) << "Cant." << endl;
    cout << string(ACCION_LEN + BOLSA_LEN + 32, '-') << endl;
    
    // LEER Y PROCESAR REGISTRO POR REGISTRO (TODO JUNTO)
    int orden = 0;
    for (int i = 0; i < totalRegistros; i++) {
        Operacion op;
        
        // Leer campos
        arch.read(op.accion, ACCION_LEN);
        arch.read(reinterpret_cast<char*>(&op.plazo), 4);
        arch.read(op.bolsa, BOLSA_LEN);
        arch.read(reinterpret_cast<char*>(&op.precioUnitario), 4);
        arch.read(reinterpret_cast<char*>(&op.cantidad), 4);
        
        if (!arch) break;
        
        // NORMALIZAR PARA MOSTRAR (creamos copias con null terminator)
        char accionMostrar[ACCION_LEN + 1] = {0};
        char bolsaMostrar[BOLSA_LEN + 1] = {0};
        strncpy(accionMostrar, op.accion, ACCION_LEN);
        strncpy(bolsaMostrar, op.bolsa, BOLSA_LEN);
        
        // Mostrar registro
        cout << setw(ACCION_LEN + 2) << accionMostrar
             << setw(8) << op.plazo
             << setw(BOLSA_LEN + 2) << bolsaMostrar
             << setw(12) << fixed << setprecision(2) << op.precioUnitario
             << setw(10) << op.cantidad << endl;
        
        // ========== PROCESAMIENTO INMEDIATO ==========
        // Validar plazo
        if (op.plazo < 0 || op.plazo >= CANT_PLAZOS) continue;
        
        PlazoResumen& plazo = plazos[op.plazo];
        
        // Contar compras/ventas
        if (op.cantidad < 0) {
            plazo.compras++;
        } else {
            plazo.ventas++;
        }
        
        // Normalizar nombre de bolsa para guardar
        char nombreBolsa[BOLSA_LEN + 1];
        strncpy(nombreBolsa, op.bolsa, BOLSA_LEN);
        nombreBolsa[BOLSA_LEN] = '\0';
        
        // Buscar o crear bolsa
        int idxBolsa = buscarBolsa(plazo.bolsas, nombreBolsa);
        
        if (idxBolsa == -1) {
            BolsaResumen br;
            strcpy(br.nombre, nombreBolsa);
            br.montoTotal = 0;
            br.resultado = 0;
            plazo.bolsas.push_back(br);
            idxBolsa = plazo.bolsas.size() - 1;
        }
        
        BolsaResumen& bolsa = plazo.bolsas[idxBolsa];
        
        // Calcular monto
        float monto = op.precioUnitario * abs(op.cantidad);
        bolsa.montoTotal += monto;
        
        // Calcular resultado
        if (op.cantidad > 0) {
            bolsa.resultado += monto;
        } else {
            bolsa.resultado -= monto;
        }
        
        // Guardar operación
        OperacionInfo oi;
        strncpy(oi.accion, op.accion, ACCION_LEN);
        oi.accion[ACCION_LEN] = '\0';
        oi.cantidad = abs(op.cantidad);
        oi.esCompra = (op.cantidad < 0);
        oi.ordenOriginal = orden++;
        bolsa.operaciones.push_back(oi);
    }
    
    arch.close();
    cout << string(ACCION_LEN + BOLSA_LEN + 32, '-') << endl;
    cout << "Total registros leidos: " << orden << endl << endl;
    
    return plazos;
}

/* ================== ORDENAMIENTO ================== */

bool compararBolsa(const BolsaResumen& a, const BolsaResumen& b) {
    return strcmp(a.nombre, b.nombre) < 0;
}

bool compararOperacion(const OperacionInfo& a, const OperacionInfo& b) {
    return a.ordenOriginal < b.ordenOriginal;
}

/* ================== LISTADO ================== */

void listarResumen(vector<PlazoResumen>& plazos) {
    cout << "\nListado:\n" << endl;
    
    for (int p = 0; p < CANT_PLAZOS; p++) {
        PlazoResumen& plazo = plazos[p];
        
        if (plazo.compras == 0 && plazo.ventas == 0) continue;
        
        cout << "Plazo: " << plazo.nombre 
             << ", Compras: " << plazo.compras
             << ", Ventas: " << plazo.ventas << endl << endl;
        
        // Ordenar bolsas alfabéticamente
        sort(plazo.bolsas.begin(), plazo.bolsas.end(), compararBolsa);
        
        for (BolsaResumen& bolsa : plazo.bolsas) {
            cout << left;
            cout << setw(16) << "Bolsa"
                 << setw(14) << "Monto"
                 << "Resultado" << endl;
            
            cout << setw(16) << bolsa.nombre
                 << setw(14) << fixed << setprecision(2) << bolsa.montoTotal
                 << fixed << setprecision(2) << bolsa.resultado << endl << endl;
            
            cout << setw(8) << "Oper"
                 << setw(ACCION_LEN + 2) << "Accion"
                 << "Cant." << endl;
            cout << string(8 + ACCION_LEN + 2 + 8, '-') << endl;
            
            // Ordenar operaciones por orden original
            sort(bolsa.operaciones.begin(), bolsa.operaciones.end(), compararOperacion);
            
            for (const OperacionInfo& op : bolsa.operaciones) {
                cout << setw(8) << (op.esCompra ? "Cpra" : "Vta")
                     << setw(ACCION_LEN + 2) << op.accion
                     << setw(8) << op.cantidad << endl;
            }
            cout << endl;
        }
        cout << endl;
    }
}

/* ================== MAIN ================== */

int main() {
    cout << fixed;
    cout.precision(2);
    
    cout << "=== LECTURA Y PROCESAMIENTO DE DATOS.BIN ===" << endl;
    
    // LEER Y PROCESAR EN UN SOLO PASO
    vector<PlazoResumen> resumen = leerYProcesar("datos.bin");
    
    if (resumen.empty()) {
        cerr << "Error: No se pudieron cargar las operaciones." << endl;
        return 1;
    }
    
    // SOLO LISTAR (YA ESTÁ TODO PROCESADO)
    listarResumen(resumen);
    
    return 0;
}
