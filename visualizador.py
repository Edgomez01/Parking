# ============================================================================
# ARCHIVO: visualizador.py
# PROPÓSITO: Interfaz gráfica del sistema de parqueadero
# DESCRIPCIÓN: Crea una ventana con Tkinter que muestra las 40 plazas
#              de forma visual, permitiendo ocupar y liberar espacios
# ============================================================================

# IMPORTACIONES
# -------------
# tkinter: Librería estándar de Python para crear interfaces gráficas (GUI)
import tkinter as tk

# ttk: Versión "mejorada" de tkinter con widgets más modernos
from tkinter import ttk

# parking: Nuestra librería SWIG (se importa aunque no se use directamente aquí)
# El connector lo usa internamente
import parking


class ParkingVisualizer:
    """
    Clase que maneja toda la interfaz gráfica del visualizador.
    
    Componentes principales:
    - Resumen: Labels que muestran total/ocupadas/libres
    - Grid: 40 botones (8 filas x 5 columnas) representando las plazas
    - Interacción: Click en botones para ocupar/liberar plazas
    - Auto-actualización: Refresco automático cada 2 segundos
    """
    
    def __init__(self, connector):
        """
        Constructor: Inicializa la ventana y prepara los contenedores.
        
        Parámetro:
        - connector: Instancia de ParkingConnector para leer/modificar el estado
        """
        
        # GUARDAR REFERENCIA AL CONNECTOR
        # --------------------------------
        # Necesitamos el connector para:
        # 1. Leer el estado del parking (get_parking_state)
        # 2. Modificar plazas (parking_manager.addVehicle, removeVehicle)
        self.connector = connector
        
        # CREAR VENTANA PRINCIPAL
        # -----------------------
        # Tk() crea la ventana raíz de la aplicación
        # Solo debe haber UNA ventana Tk() por programa
        self.root = tk.Tk()
        
        # Establecer título de la ventana
        self.root.title("Parqueadero - Estado en Tiempo Real")
        
        # PREPARAR CONTENEDORES PARA WIDGETS
        # -----------------------------------
        # Estas listas guardarán referencias a los widgets
        # ¿Por qué? Para poder ACTUALIZAR su contenido sin recrearlos
        
        # Lista que contendrá los 40 botones (uno por plaza)
        self.spot_buttons = []
        
        # Diccionario que contendrá los labels del resumen
        # Ej: {'total': Label, 'occupied': Label, 'free': Label}
        self.summary_labels = {}
        
    def create_grid(self):
        """
        Crea la estructura visual de la interfaz (solo se llama UNA vez).
        
        ¿Por qué solo una vez?
        - Crear y destruir widgets repetidamente causa parpadeo y lentitud
        - En lugar de eso, creamos los widgets una vez y luego solo
          cambiamos su texto/color con .config()
        
        Estructura:
        1. Frame de resumen (arriba)
        2. Grid de 40 botones (8 filas x 5 columnas)
        """
        
        # OBTENER INFORMACIÓN INICIAL
        # ---------------------------
        parking_state = self.connector.get_parking_state()
        total_spots = parking_state['total_spots']  # Siempre 40
        
        # ======================================================================
        # SECCIÓN 1: CREAR FRAME DE RESUMEN
        # ======================================================================
        
        # Crear un "contenedor" para los labels de resumen
        # Frame = contenedor invisible que agrupa widgets
        summary_frame = ttk.Frame(self.root)
        
        # pack() = "empaqueta" el frame en la ventana
        # pady=10 = deja 10 píxeles de espacio vertical arriba y abajo
        summary_frame.pack(pady=10)
        
        # CREAR LABELS DE RESUMEN
        # -----------------------
        # Creamos 3 labels (total, ocupadas, libres) pero los dejamos vacíos
        # refresh_display() los llenará con el texto correcto
        
        # Label para "Total de Espacios: 40"
        self.summary_labels['total'] = ttk.Label(summary_frame, text="")
        self.summary_labels['total'].grid(row=0, column=0, padx=5)
        
        # Label para "Espacios Ocupados: X"
        self.summary_labels['occupied'] = ttk.Label(summary_frame, text="")
        self.summary_labels['occupied'].grid(row=0, column=1, padx=5)
        
        # Label para "Espacios Libres: Y"
        self.summary_labels['free'] = ttk.Label(summary_frame, text="")
        self.summary_labels['free'].grid(row=0, column=2, padx=5)
        
        # ======================================================================
        # SECCIÓN 2: CREAR GRID DE BOTONES (40 PLAZAS)
        # ======================================================================
        
        # Crear otro frame para los botones de las plazas
        grid_frame = ttk.Frame(self.root)
        grid_frame.pack(pady=10)
        
        # CREAR LOS 40 BOTONES
        # --------------------
        # Usamos un bucle for para no repetir código 40 veces
        for spot_index in range(total_spots):  # 0, 1, 2, ... 39
            # Crear un botón
            spot_button = tk.Button(
                grid_frame,                # ¿Dónde? En el frame de la grid
                text="",                    # Texto inicial vacío (se actualiza después)
                bg="green",                 # Color de fondo inicial (verde = vacío)
                width=15,                   # Ancho en caracteres
                height=3,                   # Alto en líneas de texto
                font=("Arial", 9, "bold"),  # Fuente del texto
                # command = función que se ejecuta al hacer click
                # lambda = función anónima pequeña
                # idx=spot_index = captura el valor actual (importante en bucles)
                command=lambda idx=spot_index: self.on_spot_click(idx)
            )
            
            # POSICIONAR EL BOTÓN EN LA GRID
            # -------------------------------
            # Queremos 5 columnas, entonces:
            # Plaza 0: fila 0, columna 0
            # Plaza 1: fila 0, columna 1
            # ...
            # Plaza 5: fila 1, columna 0
            # ...
            # Fórmulas:
            # fila = spot_index // 5 (división entera)
            # columna = spot_index % 5 (resto de la división)
            spot_button.grid(
                row=spot_index // 5,      # Calcular fila
                column=spot_index % 5,     # Calcular columna
                padx=5,                    # Espacio horizontal entre botones
                pady=5                     # Espacio vertical entre botones
            )
            
            # GUARDAR REFERENCIA AL BOTÓN
            # ---------------------------
            # Agregamos el botón a la lista para poder actualizarlo después
            # sin tener que recrearlo
            self.spot_buttons.append(spot_button)
        
        # ======================================================================
        # SECCIÓN 3: PRIMERA ACTUALIZACIÓN
        # ======================================================================
        
        # Ahora que todos los widgets están creados, actualizar su contenido
        # (llenar los textos con la información real del parking)
        self.refresh_display()
    
    def on_spot_click(self, spot_index):
        """
        Maneja el evento de click en un botón de plaza.
        
        Lógica:
        - Si la plaza está ocupada → Liberarla
        - Si la plaza está vacía → Pedir placa y ocuparla
        
        Parámetro:
        - spot_index: Índice de la plaza clickeada (0-39)
        """
        
        # OBTENER REFERENCIA AL PARKING MANAGER
        # --------------------------------------
        # pm = parking_manager de C++ (a través de SWIG)
        pm = self.connector.parking_manager
        
        # VERIFICAR SI LA PLAZA ESTÁ OCUPADA
        # -----------------------------------
        # Llamar al método C++ isSpotOccupied
        if pm.isSpotOccupied(spot_index):
            # =========================================================
            # CASO 1: PLAZA OCUPADA → LIBERAR
            # =========================================================
            
            # Obtener la placa del vehículo en esta plaza
            plate = pm.getPlate(spot_index)
            
            # Llamar al método C++ removeVehicle
            # Retorna -1 si falla, o el índice de la plaza si tiene éxito
            result = pm.removeVehicle(plate)
            
            if result != -1:
                # Éxito: plaza liberada
                print(f"✓ Plaza {spot_index + 1} liberada (era {plate})")
                # Actualizar la pantalla inmediatamente
                self.update_display_now()
        else:
            # =========================================================
            # CASO 2: PLAZA VACÍA → OCUPAR
            # =========================================================
            
            # Pedir al usuario que ingrese la placa
            self.ask_plate_and_occupy(spot_index)
    
    def ask_plate_and_occupy(self, spot_index):
        """
        Solicita la placa al usuario y ocupa la plaza si la validación es exitosa.
        
        Flujo:
        1. Mostrar diálogo pidiendo la placa
        2. Validar formato (AAA000)
        3. Si es válida: agregar vehículo y actualizar GUI
        4. Si es inválida: mostrar mensaje de error
        
        Parámetro:
        - spot_index: Índice de la plaza a ocupar (0-39)
        """
        
        # IMPORTAR MÓDULOS DE TKINTER PARA DIÁLOGOS
        # ------------------------------------------
        # simpledialog: Para pedir texto al usuario
        # messagebox: Para mostrar alertas (éxito, error, información)
        from tkinter import simpledialog, messagebox
        
        # IMPORTAR DATETIME PARA TIMESTAMP
        # ---------------------------------
        # Necesitamos registrar fecha y hora del ingreso
        from datetime import datetime
        
        # MOSTRAR DIÁLOGO PARA PEDIR LA PLACA
        # ------------------------------------
        # askstring() muestra una ventana emergente con un campo de texto
        plate = simpledialog.askstring(
            "Ocupar Plaza",  # Título de la ventana
            # Mensaje con instrucciones
            f"Ingrese la placa para Plaza {spot_index + 1}:\n(Formato: ABC123 - 3 letras y 3 números)",
            parent=self.root  # Ventana padre (para centrar el diálogo)
        )
        
        # VERIFICAR SI EL USUARIO INGRESÓ ALGO
        # -------------------------------------
        # Si el usuario cancela, plate será None
        if plate:
            # NORMALIZAR LA PLACA
            # -------------------
            # upper() = convertir a mayúsculas (ABC123 en lugar de abc123)
            # strip() = eliminar espacios al inicio y final
            plate = plate.upper().strip()
            
            # VALIDAR FORMATO
            # ---------------
            # Llamar a la función validate_plate que verifica AAA000
            if self.validate_plate(plate):
                # ========================================================
                # PLACA VÁLIDA
                # ========================================================
                
                # OBTENER TIMESTAMP ACTUAL
                # ------------------------
                # strftime = "string from time" (convertir fecha a texto)
                # "%Y-%m-%d %H:%M:%S" = formato "2024-11-25 14:30:00"
                timestamp = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
                
                # OBTENER PARKING MANAGER
                pm = self.connector.parking_manager
                
                # INTENTAR AGREGAR EL VEHÍCULO
                # -----------------------------
                # addVehicle retorna True si tuvo éxito, False si falló
                if pm.addVehicle(spot_index, plate, timestamp):
                    # ÉXITO
                    print(f"✓ Plaza {spot_index + 1} ocupada con placa {plate}")
                    
                    # Mostrar mensaje de confirmación al usuario
                    messagebox.showinfo("Éxito", f"Vehículo {plate} estacionado en Plaza {spot_index + 1}")
                    
                    # Actualizar la pantalla inmediatamente
                    self.update_display_now()
                else:
                    # ERROR AL AGREGAR
                    print(f"✗ Error al ocupar plaza {spot_index + 1}")
                    messagebox.showerror("Error", f"No se pudo ocupar la plaza {spot_index + 1}")
            else:
                # ========================================================
                # PLACA INVÁLIDA
                # ========================================================
                
                print(f"✗ Placa inválida: {plate}")
                
                # Mostrar mensaje de error detallado
                messagebox.showerror(
                    "Placa Inválida",  # Título del error
                    # Mensaje explicando el formato correcto
                    f"La placa '{plate}' no es válida.\n\n"
                    "Formato correcto: ABC123\n"
                    "- Primeros 3 caracteres: LETRAS\n"
                    "- Últimos 3 caracteres: NÚMEROS"
                )
    
    def validate_plate(self, plate):
        """
        Valida que la placa tenga el formato correcto: AAA000
        
        Reglas:
        - Debe tener exactamente 6 caracteres
        - Primeros 3 caracteres: LETRAS (A-Z)
        - Últimos 3 caracteres: NÚMEROS (0-9)
        
        Ejemplos válidos: ABC123, XYZ789, DEF000
        Ejemplos inválidos: AB123 (muy corta), 123ABC (orden inverso), ABCD12 (4 letras)
        
        Retorna:
        - True si la placa es válida
        - False si la placa es inválida
        """
        
        # VALIDAR LONGITUD
        # ----------------
        # Una placa debe tener exactamente 6 caracteres
        if len(plate) != 6:
            return False
        
        # VALIDAR PRIMEROS 3 CARACTERES (DEBEN SER LETRAS)
        # -------------------------------------------------
        # Revisamos las posiciones 0, 1, 2
        for i in range(3):
            # isalpha() retorna True si el carácter es una letra (A-Z, a-z)
            if not plate[i].isalpha():
                return False  # Si encontramos un no-letra, la placa es inválida
        
        # VALIDAR ÚLTIMOS 3 CARACTERES (DEBEN SER NÚMEROS)
        # -------------------------------------------------
        # Revisamos las posiciones 3, 4, 5
        for i in range(3, 6):
            # isdigit() retorna True si el carácter es un dígito (0-9)
            if not plate[i].isdigit():
                return False  # Si encontramos un no-número, la placa es inválida
        
        # SI PASÓ TODAS LAS VALIDACIONES
        # -------------------------------
        # La placa tiene el formato correcto
        return True
    
    def refresh_display(self):
        """
        Actualiza solo el CONTENIDO de los widgets existentes, NO los recrea.
        
        ¿Por qué no recrear?
        - Recrear widgets causa parpadeo (flicker) en la pantalla
        - Es más lento (crear objetos consume tiempo y memoria)
        - Es innecesario (podemos cambiar texto/color de widgets existentes)
        
        Este método:
        1. Obtiene el estado actual del parking
        2. Actualiza los labels de resumen con .config()
        3. Actualiza cada botón de plaza con .config()
        """
        
        # OBTENER ESTADO ACTUAL
        # ---------------------
        # Llamar al connector para obtener toda la información del parking
        parking_state = self.connector.get_parking_state()
        
        # Extraer datos del diccionario
        total_spots = parking_state['total_spots']        # 40
        occupied_count = parking_state['occupied_count']  # Ej: 5
        free_count = parking_state['free_count']          # Ej: 35
        vehicles = parking_state['vehicles']              # Lista de vehículos
        
        # ======================================================================
        # ACTUALIZAR LABELS DE RESUMEN
        # ======================================================================
        
        # .config() = "configura" o "actualiza" un widget existente
        # Cambiamos solo el texto, el widget sigue siendo el mismo objeto
        self.summary_labels['total'].config(text=f"Total de Espacios: {total_spots}")
        self.summary_labels['occupied'].config(text=f"Espacios Ocupados: {occupied_count}")
        self.summary_labels['free'].config(text=f"Espacios Libres: {free_count}")
        
        # ======================================================================
        # ACTUALIZAR CADA BOTÓN DE PLAZA
        # ======================================================================
        
        # Iterar por todas las plazas (0 a 39)
        for spot_index in range(total_spots):
            # BUSCAR SI ESTA PLAZA TIENE UN VEHÍCULO
            # ---------------------------------------
            # next() = encuentra el primer elemento que cumple la condición
            # (v for v in vehicles if ...) = generador que busca en la lista
            # None = valor por defecto si no encuentra nada
            vehicle = next((v for v in vehicles if v['spot_index'] == spot_index), None)
            
            # Si vehicle es None, la plaza está vacía
            # Si vehicle es un dict, la plaza está ocupada
            is_occupied = vehicle is not None
            
            # DETERMINAR TEXTO Y COLOR DEL BOTÓN
            # -----------------------------------
            if is_occupied:
                # PLAZA OCUPADA
                # -------------
                color = '#FF6B6B'  # Rojo suave (hex color RGB)
                # \n = salto de línea para mostrar en 2 líneas
                # 🚗 = emoji de carro
                text = f"Plaza {spot_index + 1}\n🚗 {vehicle['plate']}"
            else:
                # PLAZA VACÍA
                # -----------
                color = '#51CF66'  # Verde suave (hex color RGB)
                # ✓ = símbolo de check
                text = f"Plaza {spot_index + 1}\n✓ VACÍO"
            
            # ACTUALIZAR EL BOTÓN
            # -------------------
            # Cambiamos texto (text) y color de fondo (bg) del botón existente
            # NO estamos creando un botón nuevo, solo modificando el que ya existe
            self.spot_buttons[spot_index].config(text=text, bg=color)
    
    def update_display_now(self):
        """
        Fuerza una actualización inmediata de la pantalla.
        
        Se llama cuando:
        - El usuario ocupa una plaza
        - El usuario libera una plaza
        
        Es simplemente un wrapper de refresh_display() para claridad del código.
        """
        self.refresh_display()
    
    def update_display(self):
        """
        Actualización automática periódica (cada 2 segundos).
        
        ¿Cómo funciona?
        1. Actualiza la pantalla (refresh_display)
        2. Programa la PRÓXIMA actualización en 2 segundos
        3. Se repite indefinidamente (recursión indirecta)
        
        Esta función se llama a sí misma a través de root.after(),
        creando un ciclo de actualizaciones automáticas.
        """
        
        # Actualizar el contenido de la pantalla ahora
        self.refresh_display()
        
        # PROGRAMAR LA SIGUIENTE ACTUALIZACIÓN
        # ------------------------------------
        # root.after(milisegundos, función) = "ejecuta esta función después de X ms"
        # 2000 ms = 2 segundos
        # self.update_display = esta misma función (recursión)
        # Esto crea un bucle: actualiza → espera 2s → actualiza → espera 2s → ...
        self.root.after(2000, self.update_display)
    
    def run(self):
        """
        Inicia la aplicación gráfica.
        
        Flujo:
        1. Crea todos los widgets (botones, labels)
        2. Programa el primer ciclo de actualización automática
        3. Entra en el "event loop" de Tkinter
        
        El event loop:
        - Espera eventos (clicks, movimientos del mouse, teclas, timers)
        - Ejecuta las funciones correspondientes
        - Actualiza la pantalla
        - Se repite hasta que se cierre la ventana
        """
        
        # PASO 1: CREAR LA INTERFAZ
        # --------------------------
        # Construir toda la estructura de widgets (botones, labels, frames)
        self.create_grid()
        
        # PASO 2: PROGRAMAR ACTUALIZACIONES AUTOMÁTICAS
        # ----------------------------------------------
        # Después de 2 segundos, llamar a update_display()
        # update_display() se reprogramará a sí misma, creando el ciclo
        self.root.after(2000, self.update_display)
        
        # PASO 3: INICIAR EL EVENT LOOP
        # ------------------------------
        # mainloop() = "bucle principal" de Tkinter
        # Este método BLOQUEA hasta que se cierre la ventana
        # Mientras está activo:
        # - Procesa eventos (clicks, timers, etc.)
        # - Mantiene la ventana abierta y respondiendo
        # - Ejecuta las funciones programadas con .after()
        self.root.mainloop()