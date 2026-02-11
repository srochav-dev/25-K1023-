#include <iostream>
#include <fstream>
#include <cstring>

using namespace std;

/* ================== ESTRUCTURAS ================== */
#pragma pack(push, 1)

struct Producto {
    char sku[10];
    char descripcion[20];
    float costoFijo;
};

struct Reparacion {
    char cliente[15];
    int tipoProducto;
    char sku[10];
    float costoDirecto;
    float presupuestado;
};

#pragma pack(pop)

/* ================== LECTURA DE ARCHIVOS ================== */

int leerProductos(const char* nombre, Producto productos[], int dim) {
    ifstream arch(nombre, ios::binary);
    if (!arch)
        return 0;

    int i = 0;
    while (i < dim &&
           arch.read(reinterpret_cast<char*>(&productos[i]), sizeof(Producto))) {

        productos[i].sku[9] = '\0';
        productos[i].descripcion[19] = '\0';
        i++;
    }

    arch.close();
    return i;
}

int contarReparaciones(const char* nombre) {
    ifstream arch(nombre, ios::binary);
    if (!arch)
        return 0;

    arch.seekg(0, ios::end);
    int bytes = arch.tellg();
    arch.close();

    return bytes / sizeof(Reparacion);
}

Reparacion* leerReparaciones(const char* nombre, int& cantidad) {
    cantidad = contarReparaciones(nombre);
    if (cantidad == 0)
        return nullptr;

    ifstream arch(nombre, ios::binary);
    if (!arch)
        return nullptr;

    Reparacion* vec = new Reparacion[cantidad];

    for (int i = 0; i < cantidad; i++) {
        arch.read(reinterpret_cast<char*>(&vec[i]), sizeof(Reparacion));

        vec[i].cliente[14] = '\0';
        vec[i].sku[9] = '\0';
    }

    arch.close();
    return vec;
}

/* ================== ORDENAMIENTO TEMPLATE ================== */

template <typename T>
void ordenar(T v[], int n, bool (*criterio)(const T&, const T&)) {
    for (int i = 0; i < n - 1; i++) {
        for (int j = i + 1; j < n; j++) {
            if (criterio(v[j], v[i])) {
                T aux = v[i];
                v[i] = v[j];
                v[j] = aux;
            }
        }
    }
}

bool criterioReparacion(const Reparacion& a, const Reparacion& b) {
    int c = strcmp(a.cliente, b.cliente);
    if (c != 0)
        return c < 0;

    if (a.tipoProducto != b.tipoProducto)
        return a.tipoProducto < b.tipoProducto;

    return strcmp(a.sku, b.sku) < 0;
}

/* ================== BUSQUEDA LINEAL ORDENADA ================== */

template <typename T, typename K>
int buscarLinealOrdenado(T v[], int n, K clave,
                         int (*criterio)(const T&, K)) {
    int i = 0;
    while (i < n && criterio(v[i], clave) < 0)
        i++;

    if (i < n && criterio(v[i], clave) == 0)
        return i;

    return -1;
}

int criterioCliente(const Reparacion& r, const char* cliente) {
    return strcmp(r.cliente, cliente);
}

int criterioProducto(const Producto& p, const char* sku) {
    return strcmp(p.sku, sku);
}

/* ================== UTILIDADES ================== */

const char* textoTipo(int t) {
    switch (t) {
        case 0: return "Electronico";
        case 1: return "Mecanico";
        case 2: return "Mecatronico";
    }
    return "";
}

void listarCliente(const char* cliente,
                   Reparacion v[], int n,
                   Producto productos[], int cantProd) {

    int pos = buscarLinealOrdenado(v, n, cliente, criterioCliente);

    if (pos == -1) {
        cout << "Cliente no encontrado." << endl;
        return;
    }

    float gananciaTotal = 0;
    int i = pos;

    while (i < n && strcmp(v[i].cliente, cliente) == 0) {

        int posProd = buscarLinealOrdenado(
            productos, cantProd,
            (const char*)v[i].sku,
            criterioProducto
        );

        float costoFijo = 0;
        char descripcion[20] = "";

        if (posProd != -1) {
            costoFijo = productos[posProd].costoFijo;
            strcpy(descripcion, productos[posProd].descripcion);
        }

        float ganancia = v[i].presupuestado -
                         (costoFijo + v[i].costoDirecto);

        gananciaTotal += ganancia;

        cout << "Cliente: " << v[i].cliente << endl;
        cout << "Tipo: " << textoTipo(v[i].tipoProducto) << endl;
        cout << "SKU: " << v[i].sku << endl;
        cout << "Producto: " << descripcion << endl;
        cout << "Costo fijo: " << costoFijo << endl;
        cout << "Costo directo: " << v[i].costoDirecto << endl;
        cout << "Presupuestado: " << v[i].presupuestado << endl;
        cout << "------------------------" << endl;

        i++;
    }

    cout << "Ganancia total del cliente: "
         << gananciaTotal << endl;
}

/* ================== MAIN ================== */

int main() {

    const int CANT_PROD = 10;
    Producto productos[CANT_PROD];

    int cantProd = leerProductos("productos.bin", productos, CANT_PROD);
    if (cantProd == 0) {
        cout << "Error al leer productos.bin" << endl;
        return 1;
    }

    int cantRep;
    Reparacion* reparaciones =
        leerReparaciones("reparaciones.bin", cantRep);

    if (!reparaciones) {
        cout << "Error al leer reparaciones.bin" << endl;
        return 1;
    }

    ordenar(reparaciones, cantRep, criterioReparacion);

    char cliente[15];

    while (true) {
        cout << "\nIngrese cliente (EOF para terminar): ";
        if (!(cin >> cliente))
            break;

        listarCliente(cliente, reparaciones,
                      cantRep, productos, cantProd);
    }

    delete[] reparaciones;
    return 0;
}
