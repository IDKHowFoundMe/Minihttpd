# MiniHTTP Server
**Emilia Moreno**


Servidor web HTTP/1.1 para Linux, modular y simple, escrito en C. Usa epoll con sockets no bloqueantes y una arquitectura event-driven para manejar múltiples clientes concurrentes, donde la request solo soporta el método GET.

## Características

- Arquitectura orientada a eventos con epoll
- Sockets no bloqueantes
- Servidor de archivos estáticos desde www/
- Detección de tipo MIME según la extensión
- Validación de la petición (método, ruta, protocolo y cabecera Host)

## Limitaciones
- Solo método GET
- Sin HTTPS/TLS
- Sin conexiones persistentes (*keep-alive*): se cierra la conexión tras cada respuesta

## Estructura del proyecto

```
.
├── src/
│   ├── main.c             # main
│   ├── server.c           # Socket, epoll y manejo de clientes
│   ├── http.c             # Parseo de la petición y armado de la respuesta
│   ├── files.c            # Lectura de archivos estáticos
│   ├── mime.c             # Tipos MIME
│   └── serverMonolito.c   # Versión 1 monolito para entender el diseño inicial
├── include/               # Cabeceras (.h)
├── www/                   # Raíz de archivos estáticos 
└── Makefile               # Automatizador de compilación
```

**Requisitos**
- Linux con gcc y make

**Compilación y ejecución**

El proyecto incluye un `Makefile` para automatizar todo:
*En la carpeta raiz:*
```bash
# Compilar
make

# Ejecutar 
# escucha en el puerto 8080
./minihttp

# Limpiar binarios
make clean
```

Para detener el servidor: `Ctrl+C`.

**Pruebas**

Desde el navegador:

```
http://localhost:8080/index.html
```

Desde la terminal:

```bash
curl -i http://localhost:8080/index.html
```


**Salida esperada del servidor**

```
MiniHTTP Server
FD Server (fd: 3)
FD Epoll 4
Nuevo cliente (fd: 5)
Mensaje recibido (fd: 5):
GET /index.html HTTP/1.1
Host: localhost:8080
User-Agent: curl/8.5.0
Accept: */*

Método: GET
Ruta: /index.html
Protocolo: HTTP/1.1
Host validado
Cliente respondido y desconectado (fd: 5)
```

**Cómo funciona**

1. Se crea el socket de escucha y se registra en epoll.
2. El bucle de eventos espera actividad con epoll_wait.
3. Si hay un cliente nuevo, se acepta con accept y se registra en epoll.
4. Si un cliente envía datos, se lee y se parsea la petición.
5. Se busca su ruta con realpath() y se valida que el archivo este dentro de www/, se determina su MIME y se envía la respuesta al cliente.
6. Se cierra la conexión y se elimina el fd de epoll.



