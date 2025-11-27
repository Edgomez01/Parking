# ============================================================================
# ARCHIVO: parking_connector.py
# PROPÓSITO: Conecta el visualizador Python con el sistema de parking
# DESCRIPCIÓN: Esta clase actúa como "puente" entre:
#              1. La librería SWIG (parking) - gestiona el estado en memoria
#              2. El servidor C++ (opcional) - recibe actualizaciones en tiempo real
# ============================================================================

# IMPORTACIONES
# -------------
# socket: Para comunicación de red (conectar al servidor C++)
import socket

# parking: La librería que creamos con SWIG desde C++
# Nos permite usar la clase ParkingManager desde Python
import parking

# threading: Para manejar hilos (no se usa directamente aquí, pero se importó)
import threading

# datetime: Para obtener fecha y hora actual
from datetime import datetime


class ParkingConnector:
    """
    Clase que conecta todos los componentes del sistema.
    
    Responsabilidades:
    - Crear y gestionar una instancia de ParkingManager (librería SWIG)
    - Conectarse al servidor C++ (opcional)
    - Escuchar mensajes del servidor
    - Proporcionar el estado del parking al visualizador
    """
    
    def __init__(self):
        """
        Constructor: Se ejecuta al crear un objeto ParkingConnector.
        
        Inicializa:
        - parking_manager: Instancia de la clase C++ a través de SWIG
        - sock: Socket para conectar al servidor (inicialmente None)
        """
        
        # CREAR INSTANCIA DE LA LIBRERÍA SWIG
        # ------------------------------------
        # parking.ParkingManager() llama al constructor de la clase C++
        # Esto crea un objeto en memoria que gestiona las 40 plazas
        # ¡Es código C++ ejecutándose desde Python gracias a SWIG!
        self.parking_manager = parking.ParkingManager()
        
        # INICIALIZAR SOCKET
        # ------------------
        # El socket nos permitirá conectarnos al servidor C++
        # Inicialmente es None (sin conexión)
        self.sock = None
        
    def connect_to_server(self):
        """
        Intenta conectarse al servidor C++ en localhost:8080.
        
        ¿Qué hace?
        1. Crea un socket TCP/IP
        2. Intenta conectar a 127.0.0.1 (localhost) puerto 8080
        3. Si falla, lanza una excepción
        
        NOTA: El servidor actual solo acepta un cliente a la vez.
        Por eso el visualizador funciona en modo local.
        """
        
        # CREAR SOCKET
        # ------------
        # AF_INET = Familia de direcciones IPv4
        # SOCK_STREAM = Tipo TCP (conexión confiable y ordenada)
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        
        # CONECTAR AL SERVIDOR
        # --------------------
        # 'localhost' o '127.0.0.1' = Esta misma computadora
        # 8080 = Puerto donde el servidor está escuchando
        # Si el servidor no está corriendo, esto lanzará una excepción
        self.sock.connect(('localhost', 8080))
    
    def listen_updates(self):
        """
        Escucha actualizaciones del servidor en tiempo real.
        
        ¿Cómo funciona?
        1. Intenta conectar al servidor C++
        2. Si conecta: escucha mensajes en un bucle infinito
        3. Si NO conecta: imprime mensaje y termina (modo local)
        
        Este método se ejecuta en un hilo separado para no bloquear la GUI.
        """
        
        # BLOQUE TRY-EXCEPT PRINCIPAL
        # ---------------------------
        # Intenta conectar al servidor, pero si falla, no rompe el programa
        try:
            # Intentar conectarse
            self.connect_to_server()
            print("✓ Conectado al servidor en puerto 8080")
            
            # BUCLE DE ESCUCHA
            # ----------------
            # Ciclo infinito que escucha mensajes del servidor
            while True:
                try:
                    # RECIBIR DATOS
                    # -------------
                    # recv(1024) = "Recibe hasta 1024 bytes de datos"
                    # Este método BLOQUEA hasta que lleguen datos
                    data = self.sock.recv(1024)
                    
                    # VERIFICAR SI EL SERVIDOR CERRÓ LA CONEXIÓN
                    # -------------------------------------------
                    # Si data está vacío, significa que el servidor cerró
                    if not data:
                        break
                    
                    # DECODIFICAR MENSAJE
                    # -------------------
                    # Los datos llegan como bytes, los convertimos a texto
                    message = data.decode('utf-8').strip()
                    print(f"📨 Mensaje del servidor: {message}")
                    
                    # PARSEAR Y ACTUALIZAR PARKING_MANAGER
                    # -------------------------------------
                    # El servidor envía mensajes en formato "PLAZA:PLACA:TIMESTAMP"
                    # Ejemplo: "15:ABC123:2024-11-25 14:30:45"
                    
                    # Verificar si el mensaje tiene el formato correcto
                    if ':' in message:
                        try:
                            # SEPARAR EL MENSAJE
                            # ------------------
                            # split(':') divide el string en el carácter ':'
                            # Ejemplo: "15:ABC123:2024-11-25 14:30:45".split(':')
                            #          → ["15", "ABC123", "2024-11-25 14", "30", "45"]
                            # Necesitamos unir las últimas 3 partes para el timestamp
                            parts = message.split(':')
                            spot_str = parts[0]  # "15"
                            plate = parts[1] if len(parts) > 1 else ""      # "ABC123"
                            
                            # Reconstruir timestamp (puede tener ':' en HH:MM:SS)
                            timestamp = ':'.join(parts[2:]) if len(parts) > 2 else datetime.now().strftime("%Y-%m-%d %H:%M:%S")
                            
                            # CONVERTIR PLAZA A NÚMERO
                            # -------------------------
                            # El servidor envía plazas 1-40, pero internamente usamos 0-39
                            spot_num = int(spot_str)  # Convertir "15" a 15
                            spot_index = spot_num - 1  # Restar 1 para obtener índice (14)
                            
                            # VERIFICAR SI LA PLAZA ESTÁ EN RANGO VÁLIDO
                            # -------------------------------------------
                            if 0 <= spot_index < 40:
                                # ACTUALIZAR EL PARKING_MANAGER
                                # ------------------------------
                                # El timestamp ya viene del mensaje parseado arriba
                                
                                # Verificar si la plaza ya está ocupada
                                if self.parking_manager.isSpotOccupied(spot_index):
                                    # Plaza ocupada → Verificar si es la misma placa
                                    current_plate = self.parking_manager.getPlate(spot_index)
                                    if current_plate == plate:
                                        # Misma placa → Liberar (SALIDA)
                                        self.parking_manager.removeVehicle(plate)
                                        print(f"🚗→ Plaza {spot_num} liberada (era {plate})")
                                    else:
                                        # Placa diferente → Reemplazar (nueva placa en plaza ocupada)
                                        self.parking_manager.removeVehicle(current_plate)
                                        self.parking_manager.addVehicle(spot_index, plate, timestamp)
                                        print(f"🔄 Plaza {spot_num} cambió: {current_plate} → {plate}")
                                else:
                                    # Plaza vacía → Ocupar (ENTRADA)
                                    self.parking_manager.addVehicle(spot_index, plate, timestamp)
                                    print(f"🚗← Plaza {spot_num} ocupada con {plate}")
                            else:
                                print(f"⚠ Plaza fuera de rango: {spot_num}")
                                
                        except Exception as e:
                            print(f"✗ Error al parsear mensaje '{message}': {e}")
                    
                except Exception as e:
                    # Si hay error al recibir datos, salir del bucle
                    print(f"Error al recibir: {e}")
                    break
                    
        except Exception as e:
            # MANEJO DE ERROR DE CONEXIÓN
            # ---------------------------
            # Si no se puede conectar, el sistema funciona en "modo local"
            # Esto es útil para desarrollar/probar sin tener el servidor corriendo
            print(f"⚠ No se pudo conectar al servidor: {e}")
            print("📝 Modo local: Puedes usar el visualizador sin servidor")
            print("   Los cambios solo se guardarán en memoria local")
    
    def get_parking_state(self):
        """
        Obtiene el estado completo del parqueadero desde la librería SWIG.
        
        Retorna un diccionario con:
        - total_spots: Número total de plazas (siempre 40)
        - occupied_count: Cuántas plazas están ocupadas
        - free_count: Cuántas plazas están libres
        - vehicles: Lista de vehículos estacionados con su plaza y placa
        
        Este método es llamado por el visualizador cada vez que necesita
        actualizar la pantalla.
        """
        
        # OBTENER ESTADÍSTICAS GENERALES
        # --------------------------------
        # Estas funciones llaman directamente a los métodos de C++
        # gracias a SWIG que genera automáticamente los "wrappers"
        total_spots = self.parking_manager.getTotalSpots()        # Retorna 40
        occupied_count = self.parking_manager.getOccupiedCount()  # Cuenta ocupadas
        free_count = self.parking_manager.getFreeCount()          # Cuenta vacías
        
        # CREAR DICCIONARIO DE ESTADO
        # ---------------------------
        # Usamos un diccionario (dict) para organizar la información
        # Es más fácil de usar que devolver 4 valores separados
        parking_state = {
            'total_spots': total_spots,
            'occupied_count': occupied_count,
            'free_count': free_count,
            'vehicles': []  # Lista vacía que llenaremos a continuación
        }
        
        # OBTENER INFORMACIÓN DE CADA VEHÍCULO
        # -----------------------------------
        # Iteramos por todas las plazas (0 a 39)
        for spot_index in range(total_spots):
            # Preguntar si esta plaza está ocupada
            # (llama al método C++ isSpotOccupied)
            if self.parking_manager.isSpotOccupied(spot_index):
                # Si está ocupada, obtener la placa
                # (llama al método C++ getPlate)
                plate = self.parking_manager.getPlate(spot_index)
                
                # Agregar este vehículo a la lista
                parking_state['vehicles'].append({
                    'spot_index': spot_index,  # Índice de la plaza (0-39)
                    'plate': plate              # Placa del vehículo (ej: "ABC123")
                })
        
        # RETORNAR DICCIONARIO COMPLETO
        # -----------------------------
        # El visualizador usará este diccionario para actualizar la GUI
        return parking_state