#include <iostream>
#include <fstream>
#include <cstring>
#include <string>

using namespace std;

/* ================== CONSTANTES ================== */
const int SKU_LEN = 10;
const int DESC_LEN = 20;
const int CLIENTE_LEN = 15;
const int CANT_PROD_FIJA = 10;

/* ================== ESTRUCTURAS ================== */
// SIN #pragma pack - usamos lectura campo por campo
struct Producto {
    char sku[SKU_LEN + 1];
    char descripcion[DESC_LEN + 1];
    float costoFijo;
};

struct Reparacion {
    char cliente[CLIENTE_LEN + 1];
    int tipoProducto;
    char sku[SKU_LEN + 1];
    float costoDirecto;
    float presupuestado;
};

/* ================== LECTURA DE ARCHIVOS ================== */

bool leerProductos(const char* nombre, Producto productos[]) {
    ifstream arch(nombre, ios::binary);
    if (!arch) {
        cerr << "Error: No se pudo abrir " << nombre << endl;
        return false;
    }

    // Leer los 10 registros UNO POR UNO
    for (int i = 0; i < CANT_PROD_FIJA; i++) {
        // SKU: 10 bytes
        arch.read(productos[i].sku, SKU_LEN);
        productos[i].sku[SKU_LEN] = '\0';
        
        // Descripción: 20 bytes
        arch.read(productos[i].descripcion, DESC_LEN);
        productos[i].descripcion[DESC_LEN] = '\0';
        
        // Costo Fijo: 4 bytes
        arch.read(reinterpret_cast<char*>(&productos[i].costoFijo), 4);
        
        if (!arch) {
            cerr << "Error leyendo producto " << i << endl;
            arch.close();
            return false;
        }
    }

    arch.close();
    cout << "Productos cargados: " << CANT_PROD_FIJA << endl;
    return true;
}

Reparacion* leerReparaciones(const char* nombre, int& cantidad) {
    ifstream arch(nombre, ios::binary);
    if (!arch) {
        cerr << "Error: No se pudo abrir " << nombre << endl;
        return nullptr;
    }
    
    // Calcular cantidad de registros (37 bytes cada uno)
    arch.seekg(0, ios::end);
    int bytes = arch.tellg();
    arch.seekg(0, ios::beg);
    
    cantidad = bytes / 37;  // 15 + 4 + 10 + 4 + 4 = 37 bytes
    
    if (cantidad == 0) {
        cerr << "Error: No hay reparaciones para leer." << endl;
        arch.close();
        return nullptr;
    }
    
    Reparacion* vec = new Reparacion[cantidad];
    
    // Leer registro por registro
    for (int i = 0; i < cantidad; i++) {
        // Cliente: 15 bytes
        arch.read(vec[i].cliente, CLIENTE_LEN);
        vec[i].cliente[CLIENTE_LEN] = '\0';
        
        // Tipo producto: 4 bytes
        arch.read(reinterpret_cast<char*>(&vec[i].tipoProducto), 4);
        
        // SKU: 10 bytes
        arch.read(vec[i].sku, SKU_LEN);
        vec[i].sku[SKU_LEN] = '\0';
        
        // Costo Directo: 4 bytes
        arch.read(reinterpret_cast<char*>(&vec[i].costoDirecto), 4);
        
        // Presupuestado: 4 bytes
        arch.read(reinterpret_cast<char*>(&vec[i].presupuestado), 4);
        
        if (!arch) {
            cerr << "Error leyendo reparación " << i << endl;
            delete[] vec;
            cantidad = 0;
            arch.close();
            return nullptr;
        }
    }

    arch.close();
    cout << "Reparaciones cargadas: " << cantidad << endl;
    return vec;
}

/* ================== ORDENAMIENTO TEMPLATE ================== */

template <typename T>
void ordenar(T v[], int n, bool (*criterio)(const T&, const T&)) {
    if (!v || n <= 1) return;
    
    bool intercambio;
    for (int i = 0; i < n - 1; i++) {
        intercambio = false;
        for (int j = 0; j < n - i - 1; j++) {
            if (criterio(v[j + 1], v[j])) {
                T temp = v[j];
                v[j] = v[j + 1];
                v[j + 1] = temp;
                intercambio = true;
            }
        }
        if (!intercambio) break;
    }
}

bool criterioReparacion(const Reparacion& a, const Reparacion& b) {
    // 1. Cliente
    int cmpCliente = strcmp(a.cliente, b.cliente);
    if (cmpCliente != 0) return cmpCliente < 0;
    
    // 2. Tipo de producto
    if (a.tipoProducto != b.tipoProducto) 
        return a.tipoProducto < b.tipoProducto;
    
    // 3. SKU
    return strcmp(a.sku, b.sku) < 0;
}

/* ================== BUSQUEDA LINEAL ORDENADA ================== */

template <typename T, typename K>
int buscarLinealOrdenado(const T v[], int n, K clave,
                         int (*criterio)(const T&, K)) {
    for (int i = 0; i < n; i++) {
        int cmp = criterio(v[i], clave);
        if (cmp == 0) {
            return i;
        }
        if (cmp > 0) {
            return -1;
        }
    }
    return -1;
}

int criterioCliente(const Reparacion& r, const char* cliente) {
    return strcmp(r.cliente, cliente);
}

/* ================== BUSQUEDA DE PRODUCTO ================== */

int buscarProductoBinario(const Producto v[], int n, const char* sku) {
    int izq = 0, der = n - 1;
    
    while (izq <= der) {
        int med = izq + (der - izq) / 2;
        int cmp = strcmp(sku, v[med].sku);
        
        if (cmp == 0) return med;
        if (cmp < 0) der = med - 1;
        else izq = med + 1;
    }
    return -1;
}

/* ================== UTILIDADES ================== */

const char* textoTipo(int t) {
    switch (t) {
        case 0: return "Electronico";
        case 1: return "Mecanico";
        case 2: return "Mecatronico";
        default: return "Desconocido";
    }
}

void listarCliente(const char* cliente,
                   const Reparacion v[], int n,
                   const Producto productos[], int cantProd) {

    int pos = buscarLinealOrdenado(v, n, cliente, criterioCliente);

    if (pos == -1) {
        cout << "Cliente no encontrado: " << cliente << endl;
        return;
    }

    float gananciaTotal = 0.0f;
    int i = pos;
    bool hayReparaciones = false;

    cout << "\n=========================================" << endl;
    cout << "REPARACIONES DE: " << cliente << endl;
    cout << "=========================================" << endl;

    while (i < n && strcmp(v[i].cliente, cliente) == 0) {
        hayReparaciones = true;
        
        int posProd = buscarProductoBinario(productos, cantProd, v[i].sku);
        
        float costoFijo = 0.0f;
        const char* descripcion = "NO ENCONTRADO";
        
        if (posProd != -1) {
            costoFijo = productos[posProd].costoFijo;
            descripcion = productos[posProd].descripcion;
        }
        
        float ganancia = v[i].presupuestado - (costoFijo + v[i].costoDirecto);
        gananciaTotal += ganancia;

        cout << "-----------------------------------------" << endl;
        cout << "Cliente: " << v[i].cliente << endl;
        cout << "Tipo Producto: " << textoTipo(v[i].tipoProducto) << endl;
        cout << "SKU: " << v[i].sku << endl;
        cout << "Producto: " << descripcion << endl;
        cout << "Costo Fijo: $" << fixed << costoFijo << endl;
        cout << "Costo Directo: $" << fixed << v[i].costoDirecto << endl;
        cout << "Presupuestado: $" << fixed << v[i].presupuestado << endl;
        cout << "Ganancia: $" << fixed << ganancia << endl;

        i++;
    }

    if (hayReparaciones) {
        cout << "-----------------------------------------" << endl;
        cout << "GANANCIA TOTAL CON CLIENTE: $" << fixed << gananciaTotal << endl;
        cout << "=========================================" << endl;
    }
}

/* ================== MAIN ================== */

int main() {
    // Configurar formato de salida
    cout << fixed;
    cout.precision(2);

    // 1. Cargar productos
    Producto productos[CANT_PROD_FIJA];
    if (!leerProductos("productos.bin", productos)) {
        cerr << "Error fatal: No se pudieron cargar los productos." << endl;
        return 1;
    }

    // 2. Cargar reparaciones
    int cantRep;
    Reparacion* reparaciones = leerReparaciones("reparaciones.bin", cantRep);
    if (!reparaciones) {
        cerr << "Error fatal: No se pudieron cargar las reparaciones." << endl;
        return 1;
    }

    // 3. Ordenar reparaciones
    cout << "Ordenando reparaciones..." << endl;
    ordenar(reparaciones, cantRep, criterioReparacion);
    cout << "Reparaciones ordenadas correctamente." << endl;

    // 4. Ciclo de consultas
    cout << "\n=== SISTEMA DE CONSULTA DE REPARACIONES ===" << endl;
    cout << "Ingrese nombre de cliente (Ctrl+Z + Enter para terminar):" << endl;
    
    string linea;
    int consultas = 0;
    
    while (getline(cin, linea)) {
        // Ignorar líneas vacías
        if (linea.empty()) continue;
        
        // Trim espacios en blanco
        size_t first = linea.find_first_not_of(" \t");
        size_t last = linea.find_last_not_of(" \t");
        
        if (first == string::npos) continue;
        
        string cliente_trim = linea.substr(first, last - first + 1);
        
        // Limitar a 15 caracteres
        char cliente_busqueda[CLIENTE_LEN + 1] = {0};
        strncpy(cliente_busqueda, cliente_trim.c_str(), CLIENTE_LEN);
        
        // Buscar y listar
        listarCliente(cliente_busqueda, reparaciones, cantRep, productos, CANT_PROD_FIJA);
        consultas++;
        
        cout << "\nIngrese otro cliente (Ctrl+Z + Enter para terminar):" << endl;
    }
    
    cout << "\n=========================================" << endl;
    cout << "Programa terminado." << endl;
    cout << "Consultas realizadas: " << consultas << endl;
    cout << "=========================================" << endl;

    // Liberar memoria
    delete[] reparaciones;
    
    return 0;
}
