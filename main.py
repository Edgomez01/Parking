# ============================================================================
# ARCHIVO: main.py
# PROPÓSITO: Punto de entrada principal del sistema de parqueadero
# DESCRIPCIÓN: Este archivo inicia todos los componentes del sistema:
#              - Connector (comunicación con servidor)
#              - Visualizador (interfaz gráfica)
# ============================================================================

# IMPORTACIONES
# -------------
# ParkingConnector: Clase que maneja la conexión al servidor y la librería SWIG
from parking_connector import ParkingConnector

# ParkingVisualizer: Clase que crea la ventana gráfica con las 40 plazas
from visualizador import ParkingVisualizer

# threading: Permite ejecutar código en paralelo (multihilo)
# Lo necesitamos para que la conexión al servidor no bloquee la GUI
import threading


def main():
    """
    Función principal que inicia el sistema completo.
    
    Flujo de ejecución:
    1. Crea el conector (que gestiona la librería SWIG)
    2. Inicia la conexión al servidor en un hilo separado
    3. Espera un segundo para dar tiempo a la conexión
    4. Abre la ventana del visualizador
    """
    
    # Mostrar mensaje de bienvenida al usuario
    print("=== Sistema de Visualización de Parqueadero ===")
    print("Asegúrate de que el servidor esté ejecutándose (servidor.cpp)")
    print()
    
    # PASO 1: Crear el conector
    # --------------------------
    # El conector gestiona:
    # - La librería SWIG (parking_manager) que guarda el estado de las plazas
    # - La conexión opcional al servidor C++
    connector = ParkingConnector()
    
    # PASO 2: Iniciar conexión al servidor en un hilo separado
    # ----------------------------------------------------------
    # ¿Por qué un thread (hilo) separado?
    # - Si intentamos conectar en el hilo principal, bloquearía la GUI
    # - Con threading, la conexión se ejecuta en paralelo
    # - La GUI puede abrirse incluso si el servidor no está disponible
    thread = threading.Thread(target=connector.listen_updates)
    
    # daemon=True significa:
    # "Cuando el programa principal termine, este hilo también debe terminar"
    # Sin esto, el programa no se cerraría correctamente
    thread.daemon = True
    
    # Iniciar el hilo (comenzar a ejecutar listen_updates en paralelo)
    thread.start()
    
    # PASO 3: Dar tiempo para que se conecte
    # ---------------------------------------
    # Importamos time solo aquí porque solo lo necesitamos una vez
    import time
    # Pausar 1 segundo para dar tiempo a que el thread intente conectarse
    # Si el servidor está disponible, se habrá conectado en este tiempo
    time.sleep(1)
    
    # PASO 4: Crear y mostrar la ventana del visualizador
    # ----------------------------------------------------
    print("Abriendo visualizador...")
    
    # Crear el visualizador, pasándole el connector
    # El visualizador necesita el connector para leer/modificar el estado del parking
    visualizer = ParkingVisualizer(connector)
    
    # run() abre la ventana y entra en el "loop de eventos" de Tkinter
    # Este método NO retorna hasta que se cierre la ventana
    # Es aquí donde el programa pasa la mayor parte del tiempo
    visualizer.run()
    
    # Cuando llegamos aquí, significa que el usuario cerró la ventana
    # El programa terminará automáticamente


# PUNTO DE ENTRADA DEL PROGRAMA
# ------------------------------
# Esta es una convención de Python:
# __name__ == "__main__" es True solo cuando ejecutas este archivo directamente
# Si alguien importara este archivo, main() NO se ejecutaría automáticamente
if __name__ == "__main__":
    main()