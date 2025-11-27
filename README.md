# Parking
Parqueadero, estacionamiento

# 📘 GUÍA COMPLETA DE INSTALACIÓN Y COMPILACIÓN
## Sistema de Gestión de Parqueadero con C++ y Python

---

## 📋 TABLA DE CONTENIDOS

1. [Requisitos Previos](#requisitos-previos)
2. [Instalación de Visual Studio 2022](#instalación-de-visual-studio-2022)
3. [Instalación de Python](#instalación-de-python)
4. [Instalación de SWIG](#instalación-de-swig)
5. [Compilación de la Librería SWIG](#compilación-de-la-librería-swig)
6. [Compilación del Servidor y Cliente](#compilación-del-servidor-y-cliente)
7. [Ejecución del Sistema Completo](#ejecución-del-sistema-completo)
8. [Solución de Problemas Comunes](#solución-de-problemas-comunes)

---

## 📌 REQUISITOS PREVIOS

### Software Necesario

| Software | Versión Recomendada | Propósito |
|----------|---------------------|-----------|
| **Windows** | 10/11 (64-bit) | Sistema operativo |
| **Visual Studio 2022** | Community Edition | Compilador C++ (MSVC) |
| **Python** | 3.10 o superior | Ejecutar el visualizador |
| **SWIG** | 4.0 o superior | Generar bindings C++→Python |

### Espacio en Disco

- **Visual Studio 2022**: ~7 GB (con componentes necesarios)
- **Python 3.12**: ~100 MB
- **SWIG**: ~20 MB
- **Total estimado**: ~8 GB

---

## 🔧 INSTALACIÓN DE VISUAL STUDIO 2022

### Paso 1: Descargar Visual Studio 2022

1. Ir a: [https://visualstudio.microsoft.com/es/downloads/](https://visualstudio.microsoft.com/es/downloads/)
2. Descargar **Visual Studio 2022 Community** (es gratuito)
3. Ejecutar el instalador descargado (`VisualStudioSetup.exe`)

### Paso 2: Seleccionar Componentes

Durante la instalación, **DEBES SELECCIONAR** estos componentes:

#### ✅ Carga de Trabajo Principal

**"Desarrollo para el escritorio con C++"**

- Marca esta opción en la pestaña "Cargas de trabajo"
- Esto instala:
  - Compilador MSVC (cl.exe)
  - Herramientas de compilación x64
  - Windows SDK
  - Bibliotecas estándar de C++

#### ✅ Componentes Individuales Adicionales

En la pestaña "Componentes individuales", buscar y marcar:

1. **MSVC v143 - VS 2022 C++ x64/x86 build tools** (latest)
2. **Windows 10 SDK** (10.0.19041.0 o superior)
3. **C++ CMake tools for Windows**
4. **C++ core features**

### Paso 3: Completar la Instalación

1. Click en "Instalar" (esquina inferior derecha)
2. Esperar ~30-60 minutos (depende de la conexión)
3. Reiniciar el sistema si se solicita

### Paso 4: Verificar la Instalación

1. Abrir el **Menú Inicio**
2. Buscar: `x64 Native Tools Command Prompt for VS 2022`
3. Abrir esta terminal
4. Ejecutar:
   ```cmd
   cl
   ```
5. Deberías ver:
   ```
   Microsoft (R) C/C++ Optimizing Compiler Version 19.XX.XXXXX for x64
   ```

❗ **IMPORTANTE**: Todos los comandos de compilación DEBEN ejecutarse en **"x64 Native Tools Command Prompt"**, NO en PowerShell ni CMD normal.

---

## 🐍 INSTALACIÓN DE PYTHON

### Paso 1: Descargar Python

1. Ir a: [https://www.python.org/downloads/](https://www.python.org/downloads/)
2. Descargar **Python 3.12** (o 3.10+)
3. Ejecutar el instalador

### Paso 2: Configurar la Instalación

⚠️ **MUY IMPORTANTE**:

1. ✅ **Marcar**: "Add Python to PATH"
2. Click en "Install Now"
3. Esperar la instalación

### Paso 3: Verificar Python

Abrir PowerShell y ejecutar:

```powershell
python --version
```

Deberías ver:
```
Python 3.12.X
```

### Paso 4: Instalar Tkinter (si no está)

Tkinter viene incluido con Python en Windows, pero verifica:

```powershell
python -c "import tkinter; print('Tkinter OK')"
```

Si ves `Tkinter OK`, está listo. Si hay error, reinstala Python marcando "tcl/tk and IDLE".

---

## 🔗 INSTALACIÓN DE SWIG

### Opción A: Descarga Directa (Recomendado)

1. Ir a: [http://www.swig.org/download.html](http://www.swig.org/download.html)
2. Descargar: **swigwin-4.x.x.zip** (versión para Windows)
3. Extraer el archivo ZIP a: `C:\swig\`
4. Agregar a PATH:
   - Abrir "Variables de entorno"
   - Editar "Path" del usuario
   - Agregar: `C:\swig\swigwin-4.x.x`
   - Click "Aceptar"

### Verificar SWIG

Abrir PowerShell nuevo y ejecutar:

```powershell
swig -version
```

Deberías ver:
```
SWIG Version 4.x.x
```

---

## 🏗️ COMPILACIÓN DE LA LIBRERÍA SWIG

Esta librería permite que Python use el código C++ del sistema de parqueadero.

### Archivos Necesarios

Asegúrate de tener estos archivos en tu carpeta:

```
Amigue/
├── parking_lib.h          (Declaraciones de la clase ParkingManager)
├── parking_lib.cpp        (Implementación de la clase ParkingManager)
├── parking.i              (Archivo de interfaz SWIG)
├── COMPILAR_LIBRERIA.bat  (Script de compilación)
```

### Paso 1: Entender los Archivos

#### `parking_lib.h` y `parking_lib.cpp`

Contienen la clase **ParkingManager** que gestiona:
- Arreglo de 40 plazas
- Funciones: `addVehicle()`, `removeVehicle()`, `isSpotOccupied()`, etc.
- Validación de placas

#### `parking.i` (Archivo de Interfaz SWIG)

Le dice a SWIG qué clases y funciones exponer a Python:

```c
%module parking
%{
#include "parking_lib.h"
%}
%include "parking_lib.h"
```

### Paso 2: Compilar la Librería

1. Abrir **"x64 Native Tools Command Prompt for VS 2022"**
2. Navegar a tu carpeta:
   ```cmd
   cd "C:\Users\USUARIO CAB\Desktop\PARQUEADERO\parking"
   ```
3. Ejecutar:
   ```cmd
   COMPILAR_LIBRERIA.bat
   ```

### Paso 3: Verificar la Compilación

Deberías ver estos archivos nuevos:

```
✅ parking_wrap.cxx        (Código wrapper generado por SWIG)
✅ parking.py              (Módulo Python generado)
✅ _parking.pyd            (Librería dinámica - equivalente a .dll)
```

El archivo **`_parking.pyd`** es el más importante: es la librería que Python cargará.

### ¿Qué Hace COMPILAR_LIBRERIA.bat?

```batch
@echo off
REM Paso 1: SWIG genera parking_wrap.cxx y parking.py
swig -c++ -python parking.i

REM Paso 2: Obtener rutas de Python
FOR /F "tokens=*" %%i IN ('python -c "import sys; print(sys.prefix)"') DO SET PYTHON_PREFIX=%%i
FOR /F "tokens=*" %%i IN ('python -c "import sys; print(sys.version_info.major)"') DO SET PYTHON_MAJOR=%%i
FOR /F "tokens=*" %%i IN ('python -c "import sys; print(sys.version_info.minor)"') DO SET PYTHON_MINOR=%%i

REM Paso 3: Compilar con MSVC
cl /LD /EHsc ^
   /I"%PYTHON_PREFIX%\include" ^
   parking_lib.cpp parking_wrap.cxx ^
   /link /LIBPATH:"%PYTHON_PREFIX%\libs" python%PYTHON_MAJOR%%PYTHON_MINOR%.lib ^
   /OUT:_parking.pyd
```

**Explicación línea por línea**:

- `swig -c++ -python parking.i`: Genera código wrapper
- `cl /LD`: Compilar como DLL (Dynamic Link Library)
- `/EHsc`: Habilitar manejo de excepciones de C++
- `/I"path\include"`: Incluir headers de Python
- `/link /LIBPATH:"path\libs"`: Vincular con python3XX.lib
- `/OUT:_parking.pyd`: Nombre de salida

### Solución de Problemas en Compilación SWIG

| Error | Solución |
|-------|----------|
| `'swig' no se reconoce` | SWIG no está en PATH. Agregar a variables de entorno |
| `'cl' no se reconoce` | No estás en x64 Native Tools Prompt |
| `fatal error C1083: Cannot open include file: 'Python.h'` | Python no encontrado. Verifica rutas en el bat |
| `_parking.pyd` no se genera | Revisar errores de compilación en pantalla |

---

## ⚙️ COMPILACIÓN DEL SERVIDOR Y CLIENTE

### Archivos del Sistema C++

```
Amigue/
├── servidor_multicliente.cpp  (Servidor con soporte multicliente)
├── cliente.cpp                (Generador automático de placas)
├── RECOMPILAR_TODO.bat        (Compila ambos archivos)
```

### Paso 1: Entender la Arquitectura

#### `servidor_multicliente.cpp`

- **Función**: Servidor TCP que acepta múltiples conexiones
- **Puerto**: 8080
- **Tecnología**: Winsock2 + threads (std::thread)
- **Capacidad**: Gestiona 40 plazas de parqueadero
- **Características**:
  - Acepta conexiones simultáneas (visualizador + generador)
  - Usa mutex para proteger datos compartidos
  - Broadcast de actualizaciones a todos los clientes

#### `cliente.cpp`

- **Función**: Generador automático de placas
- **Comportamiento**: Envía placas aleatorias cada 2-5 segundos
- **Formato de envío**: `"PLAZA:PLACA:TIMESTAMP"`
- **Ejemplo**: `"15:ABC123:2024-11-25 14:30:45"`

### Paso 2: Compilar Servidor y Cliente

1. Abrir **"x64 Native Tools Command Prompt for VS 2022"**
2. Navegar a la carpeta:
   ```cmd
   cd "C:\Users\USUARIO CAB\Desktop\PARQUEADERO\parking"
   ```
3. Ejecutar:
   ```cmd
   RECOMPILAR_TODO.bat
   ```

### Paso 3: Verificar la Compilación

Deberías ver estos ejecutables:

```
✅ servidor_multicliente.exe   (Servidor)
✅ cliente.exe                  (Generador de placas)
```

### ¿Qué Hace RECOMPILAR_TODO.bat?

```batch
@echo off
echo ========================================
echo   RECOMPILANDO SISTEMA COMPLETO
echo ========================================

REM Compilar el servidor multicliente
echo [1/2] Compilando servidor_multicliente.cpp...
cl servidor_multicliente.cpp /EHsc /Fe:servidor_multicliente.exe /link ws2_32.lib

REM Compilar el cliente generador
echo [2/2] Compilando cliente.cpp...
cl cliente.cpp /EHsc /Fe:cliente.exe /link ws2_32.lib

echo.
echo ========================================
echo   COMPILACION COMPLETADA
echo ========================================
```

**Explicación de flags**:

- `cl`: Compilador de Visual Studio
- `/EHsc`: Habilitar excepciones de C++
- `/Fe:nombre.exe`: Especificar nombre del ejecutable
- `/link ws2_32.lib`: Vincular librería de sockets de Windows

### Solución de Problemas en Compilación C++

| Error | Solución |
|-------|----------|
| `'cl' no se reconoce` | Usar x64 Native Tools Command Prompt |
| `Cannot open include file: 'WinSock2.h'` | Windows SDK no instalado. Reinstalar VS con SDK |
| `unresolved external symbol WSAStartup` | Falta `/link ws2_32.lib` |
| `error C2059: syntax error` | Revisar que el código esté completo sin errores |

---

## 🚀 EJECUCIÓN DEL SISTEMA COMPLETO

El sistema tiene **3 componentes** que deben ejecutarse en orden específico:

### Arquitectura del Sistema

```
┌─────────────────────────┐
│  servidor_multicliente  │  (Componente 1: Servidor C++)
│      Puerto 8080        │
└───────────┬─────────────┘
            │
            ├──────────────┐
            │              │
┌───────────▼─────┐  ┌────▼──────────────┐
│   main.py       │  │   cliente.exe     │
│  (Visualizador) │  │ (Generador placas)│
│     Python      │  │       C++         │
└─────────────────┘  └───────────────────┘
   Componente 2         Componente 3
```

### PASO 1: Iniciar el Servidor

**Terminal 1** - PowerShell o CMD:

```powershell
cd "C:\Users\USUARIO CAB\Desktop\PARQUEADERO\parking"
.\servidor_multicliente.exe
```

**Salida esperada**:
```
========================================================
  SERVIDOR MULTICLIENTE - PARQUEADERO
========================================================
[OK] Servidor iniciado en puerto 8080
[*] Gestiona 40 plazas
[*] Soporta MULTIPLES clientes simultaneamente
[*] Esperando conexiones...
========================================================
```

⚠️ **¡IMPORTANTE!**: Dejar esta terminal abierta. El servidor debe estar corriendo todo el tiempo.

### PASO 2: Iniciar el Visualizador

**Terminal 2** - PowerShell nueva:

```powershell
cd "C:\Users\USUARIO CAB\Desktop\STEFFI\Amigue"
python main.py
```

**Salida esperada**:
```
=== Sistema de Visualización de Parqueadero ===
Asegúrate de que el servidor esté ejecutándose (servidor.cpp)

✓ Conectado al servidor en puerto 8080
Abriendo visualizador...
```

**Se abrirá una ventana gráfica** mostrando:
- 40 botones (plazas del parqueadero)
- Resumen: Total/Ocupadas/Libres
- Botones verdes = plazas vacías
- Botones rojos = plazas ocupadas

### PASO 3: Iniciar el Generador de Placas

**Terminal 3** - PowerShell nueva:

```powershell
cd "C:\Users\USUARIO CAB\Desktop\PARQUEADERO\parking"
.\cliente.exe
```

**Salida esperada**:
```
============================================
  CLIENTE - GENERADOR AUTOMATICO DE PLACAS
============================================
[OK] Conectado al servidor en puerto 8080
[*] Generando placas automaticamente cada 2-5 segundos...
[*] Formato: AAA000 (3 letras + 3 numeros)
[*] Plaza aleatoria entre 1 y 40

>> [15] Enviando:
   Plaza: 15
   Placa: XYZ789
   Hora: 2024-11-25 14:30:45
<< OK: Vehiculo estacionado
** Esperando 3.2 segundos...
```

### 📊 Flujo de Datos

```
1. cliente.exe genera placa "XYZ789" para plaza 15
   ↓
2. Envía al servidor: "15:XYZ789:2024-11-25 14:30:45"
   ↓
3. servidor_multicliente.exe procesa:
   - Guarda en memoria: parkingSpots[14] = "XYZ789"
   - Hace broadcast a todos los clientes conectados
   ↓
4. main.py (visualizador) recibe el mensaje
   ↓
5. parking_connector.py parsea y actualiza parking_manager (SWIG)
   ↓
6. visualizador.py actualiza la GUI:
   - Plaza 15 cambia de verde a rojo
   - Muestra "🚗 XYZ789"
```

### 🔄 Liberación de Plazas

Cuando `cliente.exe` envía una **placa repetida**:

1. Servidor detecta con `findPlate()`
2. Libera la plaza: `parkingSpots[index] = nullptr`
3. Responde: `"OK: Vehiculo salio. Plaza liberada"`
4. Visualizador actualiza: botón cambia de rojo a verde

### ⏹️ Detener el Sistema

**Orden correcto de cierre**:

1. **Cerrar cliente.exe**: Presiona `Ctrl+C` en Terminal 3
2. **Cerrar visualizador**: Cierra la ventana gráfica
3. **Cerrar servidor**: Presiona `Ctrl+C` en Terminal 1

---

## 🛠️ SOLUCIÓN DE PROBLEMAS COMUNES

### Problema 1: "No se pudo conectar al servidor"

**Síntomas**:
```
⚠ No se pudo conectar al servidor: [WinError 10061]
📝 Modo local: Puedes usar el visualizador sin servidor
```

**Causas y soluciones**:

| Causa | Solución |
|-------|----------|
| Servidor no iniciado | Iniciar `servidor_multicliente.exe` primero |
| Puerto 8080 ocupado | Cerrar otros programas usando puerto 8080 |
| Firewall bloqueando | Permitir en Firewall de Windows |

### Problema 2: Servidor acepta solo un cliente

**Síntomas**: Al conectar el segundo cliente, el primero se desconecta.

**Solución**: Asegúrate de usar `servidor_multicliente.cpp`, NO `servidor.cpp` (versión antigua).

### Problema 3: "_parking.pyd no se encuentra"

**Síntomas**:
```python
ModuleNotFoundError: No module named '_parking'
```

**Solución**:
1. Verificar que `_parking.pyd` existe en la carpeta
2. Ejecutar `python main.py` desde la misma carpeta donde está el .pyd
3. Recompilar la librería SWIG

### Problema 4: Visualizador no actualiza

**Síntomas**: La ventana abre pero los botones no cambian cuando cliente.exe envía placas.

**Causas posibles**:

1. **Visualizador no conectado**:
   - Verificar salida de Terminal 2
   - Debe decir "✓ Conectado al servidor"

2. **Error en parseo**:
   - Verificar formato del mensaje en servidor
   - Debe ser "PLAZA:PLACA:TIMESTAMP"

3. **Problema con SWIG**:
   - Verificar que `parking.py` y `_parking.pyd` existen
   - Probar en Python:
     ```python
     import parking
     pm = parking.ParkingManager()
     print("SWIG funciona")
     ```

### Problema 5: Caracteres extraños en la consola

**Síntomas**: Ves `­ƒôñ` o `ÔÅ▒` en lugar de símbolos.

**Solución**: Ya está corregido en los archivos actuales (usamos ASCII en lugar de UTF-8).

### Problema 6: "Python no se reconoce"

**Síntomas**:
```
'python' no se reconoce como un comando interno o externo
```

**Solución**:
1. Reinstalar Python marcando "Add Python to PATH"
2. O usar: `py main.py` en lugar de `python main.py`

### Problema 7: Errores de compilación en C++

**Error común**:
```
fatal error C2059: syntax error: '{'
```

**Solución**:
1. Verificar que el archivo esté completo (sin truncar)
2. Revisar que no haya caracteres especiales
3. Asegurarse de usar x64 Native Tools Prompt

---

## 📝 RESUMEN DE COMANDOS IMPORTANTES

### Compilación

```batch
REM Desde x64 Native Tools Command Prompt for VS 2022

REM Compilar librería SWIG
cd "ruta\al\proyecto"
COMPILAR_LIBRERIA.bat

REM Compilar servidor y cliente
RECOMPILAR_TODO.bat
```

### Ejecución

```powershell
# Terminal 1: Servidor
.\servidor_multicliente.exe

# Terminal 2: Visualizador
python main.py

# Terminal 3: Generador
.\cliente.exe
```

### Verificaciones

```powershell
# Verificar Python
python --version

# Verificar SWIG
swig -version

# Verificar MSVC
cl

# Verificar librería SWIG
python -c "import parking; print('OK')"
```

---

## 📚 EXPLICACIÓN TÉCNICA

### ¿Por Qué Necesitamos SWIG?

**Problema**: Python no puede usar código C++ directamente.

**Solución**: SWIG genera un "wrapper" (envoltorio) que:

1. Toma funciones de C++ (`addVehicle`, `removeVehicle`)
2. Las "envuelve" en código Python-compatible
3. Compila todo en un archivo `.pyd` (similar a `.dll`)
4. Python puede importar el `.pyd` como si fuera un módulo normal

**Ejemplo visual**:

```
parking_lib.cpp (C++)          SWIG           parking.py (Python)
┌───────────────────┐          ┌──┐           ┌──────────────────┐
│ class             │          │  │           │ import parking   │
│ ParkingManager {  │  ──────> │  │  ──────>  │ pm = parking.    │
│   addVehicle()    │          │  │           │   ParkingManager()│
│   removeVehicle() │          └──┘           │ pm.addVehicle()  │
│ }                 │                          └──────────────────┘
└───────────────────┘
        │                                              │
        └──────────── _parking.pyd ───────────────────┘
              (Librería compartida compilada)
```

### ¿Por Qué Usar Threads en el Servidor?

**Sin threads (servidor original del profesor)**:
```
Cliente 1 conecta → Servidor atiende solo a Cliente 1
Cliente 2 intenta conectar → RECHAZADO (servidor ocupado)
```

**Con threads (servidor_multicliente)**:
```
Cliente 1 conecta → Thread 1 atiende Cliente 1
Cliente 2 conecta → Thread 2 atiende Cliente 2  } Simultáneamente
Visualizador conecta → Thread 3 atiende Visualizador
```

**Flujo técnico**:

1. `accept()` recibe nueva conexión
2. Crear nuevo thread: `thread(handleClient, socket)`
3. `.detach()` → Thread corre independiente
4. Thread llama `handleClient(socket)` en paralelo
5. `mutex` protege acceso a `parkingSpots[]`

### ¿Por Qué x64 Native Tools Command Prompt?

**PowerShell/CMD normal**:
- No tiene `cl.exe` en PATH
- No configura variables de entorno de VS

**x64 Native Tools Prompt**:
- Ejecuta `vcvarsall.bat` automáticamente
- Configura rutas de MSVC, Windows SDK
- Permite compilar C++ sin configuración manual

---

## ✅ CHECKLIST FINAL

Antes de ejecutar, verifica:

- [ ] Visual Studio 2022 instalado con "Desarrollo para el escritorio con C++"
- [ ] Python 3.10+ instalado con Tkinter
- [ ] SWIG en PATH (`swig -version` funciona)
- [ ] Archivos `.cpp`, `.h`, `.i` en la carpeta del proyecto
- [ ] `_parking.pyd` compilado exitosamente
- [ ] `servidor_multicliente.exe` y `cliente.exe` compilados
- [ ] 3 terminales preparadas

---

## 🎓 CONCEPTOS CLAVE EXPLICADOS

### Cliente-Servidor TCP

- **Cliente**: Inicia la conexión (connect)
- **Servidor**: Espera conexiones (listen/accept)
- **Socket**: Canal de comunicación bidireccional
- **Puerto**: Número que identifica el servicio (8080)

### Protocolo del Sistema

```
Formato: "PLAZA:PLACA:TIMESTAMP"

Ejemplos:
"15:ABC123:2024-11-25 14:30:45"  → Ocupar plaza 15
"15:ABC123:2024-11-25 15:00:00"  → Liberar plaza 15 (placa repetida)
```

### SWIG (Simplified Wrapper and Interface Generator)

- **Input**: `parking.i` (interfaz) + `parking_lib.cpp` (implementación)
- **Output**: `parking.py` + `_parking.pyd`
- **Resultado**: Python puede usar clases/funciones de C++

---

## 🆘 CONTACTO Y SOPORTE

Si encuentras problemas:

1. Revisar la sección "Solución de Problemas"
2. Verificar que todos los archivos estén presentes
3. Ejecutar comandos en el orden correcto
4. Asegurarse de usar x64 Native Tools Prompt para compilación

---

DIAGRAMA UML

<img width="887" height="545" alt="image" src="https://github.com/user-attachments/assets/3b835fce-2224-4dde-8ede-6365da131d58" />





**Última actualización**: Noviembre 2024  
**Versión del documento**: 1.0  
**Autor**: Sistema de Gestión de Parqueadero - Proyecto Académico

---
