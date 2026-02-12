Trabajo Práctico 1 – Algoritmos y Estructuras de Datos

Carrera Ingeniería en Sistemas de Información – UTN FRBA

Materia Algoritmos y Estructuras de Datos

Alumna: Sharon Ayelen Rocha Villarpando

Descripción: El programa lee un archivo binario llamado datos.bin. Cada registro representa una operación de compra o venta en diferentes bolsas y plazos. 
Lo que hace el código es procesar todo "al vuelo" mientras lee el archivo para armar un listado de 3 niveles:
Plazo: Agrupado por CI, 24hs, 48hs y 72hs.
Bolsa: Todas las bolsas donde hubo movimiento en ese plazo, ordenadas alfabéticamente.
Operaciones: El detalle de cada acción, respetando el orden original del archivo.

Cosas que usé para que funcione:
Lectura binaria exacta: El programa lee registro por registro respetando el tamaño de 38 bytes, manejando los strings sin terminador nulo que vienen en el archivo.
Procesamiento en un solo paso: Como pide el punto 2, las estructuras se van armando a medida que se lee el archivo, sin cargarlo todo en memoria primero.
Lógica de Monto y Resultado: El monto se calcula con el valor absoluto de las cantidades, pero el resultado final mantiene el signo (positivo para venta, negativo para compra).
Ordenamiento Dinámico: Usé funciones de comparación para que las bolsas aparezcan por nombre de la A a la Z antes de mostrar el listado.

Cómo lo corrí yo para probarlo en la compu usé:
g++ main.cpp -o tp2.exe
.\tp2.exe

