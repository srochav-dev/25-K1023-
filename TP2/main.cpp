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
#pragma pack(push, 1)
struct Operacion {
    char accion[ACCION_LEN];
    int plazo;
    char bolsa[BOLSA_LEN];
    float precioUnitario;
    int cantidad;
};
#pragma pack(pop)

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

/* ================== LECTURA ================== */

vector<Operacion> leerOperaciones(const char* nombre) {
    vector<Operacion> operaciones;
    
    ifstream arch(nombre, ios::binary);
    if (!arch) {
        cerr << "Error: No se pudo abrir " << nombre << endl;
        return operaciones;
    }
    
    arch.seekg(0, ios::end);
    int bytes = arch.tellg();
    arch.seekg(0, ios::beg);
    
    int cantidad = bytes / 38;
    
    cout << left;
    cout << setw(ACCION_LEN + 2) << "Accion"
         << setw(8) << "Plazo"
         << setw(BOLSA_LEN + 2) << "Bolsa"
         << setw(12) << "Pre.Uni."
         << setw(10) << "Cant." << endl;
    cout << string(ACCION_LEN + BOLSA_LEN + 32, '-') << endl;
    
    for (int i = 0; i < cantidad; i++) {
        Operacion op;
        
        arch.read(op.accion, ACCION_LEN);
        op.accion[ACCION_LEN] = '\0';
        
        arch.read(reinterpret_cast<char*>(&op.plazo), 4);
        
        arch.read(op.bolsa, BOLSA_LEN);
        op.bolsa[BOLSA_LEN] = '\0';
        
        arch.read(reinterpret_cast<char*>(&op.precioUnitario), 4);
        
        arch.read(reinterpret_cast<char*>(&op.cantidad), 4);
        
        if (!arch) break;
        
        cout << setw(ACCION_LEN + 2) << op.accion
             << setw(8) << op.plazo
             << setw(BOLSA_LEN + 2) << op.bolsa
             << setw(12) << fixed << setprecision(2) << op.precioUnitario
             << setw(10) << op.cantidad << endl;
        
        operaciones.push_back(op);
    }
    
    arch.close();
    cout << string(ACCION_LEN + BOLSA_LEN + 32, '-') << endl;
    cout << "Total registros leidos: " << operaciones.size() << endl << endl;
    
    return operaciones;
}

/* ================== ARMAR RESUMEN ================== */

vector<PlazoResumen> armarResumen(const vector<Operacion>& operaciones) {
    vector<PlazoResumen> plazos;
    
    for (int i = 0; i < CANT_PLAZOS; i++) {
        PlazoResumen p;
        p.codigo = i;
        p.nombre = NOMBRE_PLAZOS[i];
        p.compras = 0;
        p.ventas = 0;
        plazos.push_back(p);
    }
    
    for (size_t i = 0; i < operaciones.size(); i++) {
        const Operacion& op = operaciones[i];
        
        if (op.plazo < 0 || op.plazo >= CANT_PLAZOS) continue;
        
        PlazoResumen& plazo = plazos[op.plazo];
        
        if (op.cantidad < 0) {
            plazo.compras++;
        } else {
            plazo.ventas++;
        }
        
        int idxBolsa = -1;
        for (size_t j = 0; j < plazo.bolsas.size(); j++) {
            if (strcmp(plazo.bolsas[j].nombre, op.bolsa) == 0) {
                idxBolsa = j;
                break;
            }
        }
        
        if (idxBolsa == -1) {
            BolsaResumen br;
            strcpy(br.nombre, op.bolsa);
            br.montoTotal = 0;
            br.resultado = 0;
            plazo.bolsas.push_back(br);
            idxBolsa = plazo.bolsas.size() - 1;
        }
        
        BolsaResumen& bolsa = plazo.bolsas[idxBolsa];
        
        float monto = op.precioUnitario * abs(op.cantidad);
        bolsa.montoTotal += monto;
        
        if (op.cantidad > 0) {
            bolsa.resultado += monto;
        } else {
            bolsa.resultado -= monto;
        }
        
        OperacionInfo oi;
        strcpy(oi.accion, op.accion);
        oi.cantidad = abs(op.cantidad);
        oi.esCompra = (op.cantidad < 0);
        oi.ordenOriginal = i;
        bolsa.operaciones.push_back(oi);
    }
    
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

void listarResumen(const vector<PlazoResumen>& plazos) {
    cout << "\nListado:\n" << endl;
    
    for (int p = 0; p < CANT_PLAZOS; p++) {
        const PlazoResumen& plazo = plazos[p];
        
        if (plazo.compras == 0 && plazo.ventas == 0) continue;
        
        cout << "Plazo: " << plazo.nombre 
             << ", Compras: " << plazo.compras
             << ", Ventas: " << plazo.ventas << endl << endl;
        
        vector<BolsaResumen> bolsasOrdenadas = plazo.bolsas;
        sort(bolsasOrdenadas.begin(), bolsasOrdenadas.end(), compararBolsa);
        
        for (const BolsaResumen& bolsa : bolsasOrdenadas) {
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
            
            vector<OperacionInfo> opsOrdenadas = bolsa.operaciones;
            sort(opsOrdenadas.begin(), opsOrdenadas.end(), compararOperacion);
            
            for (const OperacionInfo& op : opsOrdenadas) {
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
    
    cout << "=== LECTURA DE DATOS.BIN ===" << endl;
    vector<Operacion> operaciones = leerOperaciones("datos.bin");
    
    if (operaciones.empty()) {
        cerr << "Error: No se pudieron cargar las operaciones." << endl;
        return 1;
    }
    
    vector<PlazoResumen> resumen = armarResumen(operaciones);
    listarResumen(resumen);
    
    return 0;
}