import serial
import pygame
import sys
import time

# Seriellen Port initialisieren
serial_port = "COM12"
baud_rate = 115200  # Baudrate des Arduino
ser = serial.Serial(serial_port, baud_rate, timeout=1)

# Pygame initialisieren
pygame.init()
screen_width, screen_height = 800, 600
screen = pygame.display.set_mode((screen_width, screen_height))
pygame.display.set_caption("Widerstandsanzeige")
font = pygame.font.Font(None, 74)

# Farben
black = (0, 0, 0)
white = (255, 255, 255)

# Variable für die zuletzt empfangenen Daten
last_resistance_value = None
last_update_time = time.time()

def format_resistance(value):
    """Formatiert den Widerstandswert:
       - < 1.000 Ohm: zwei Nachkommastellen
       - 1.000 bis 1.000.000 Ohm: in kOhm mit einer Nachkommastelle
       - ≥ 1.000.000 Ohm: in MOhm mit einer Nachkommastelle"""
    if value < 1000:
        return f"{value:.2f} Ohm"
    elif value < 1e6:
        return f"{value/1000:.1f} kOhm"
    else:
        return f"{value/1e6:.1f} MOhm"

# Hauptprogramm
def main():
    global last_resistance_value, last_update_time
    clock = pygame.time.Clock()
    
    while True:
        # Ereignisse prüfen (z. B. ob das Fenster geschlossen wird)
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                pygame.quit()
                ser.close()
                sys.exit()

        # Seriendaten lesen
        try:
            if ser.in_waiting > 0:
                line = ser.readline().decode('utf-8').strip()
                last_resistance_value = float(line)  # Aktualisiere den Widerstandswert
                last_update_time = time.time()         # Zeit des letzten Updates speichern
        except ValueError:
            pass  # Ungültige Werte ignorieren

        # Bildschirm aktualisieren
        screen.fill(black)  # Schwarzer Hintergrund

        # Widerstandswert anzeigen
        if last_resistance_value is not None:
            formatted_value = format_resistance(last_resistance_value)
            if time.time() - last_update_time <= 1:
                text = f"Widerstand: {formatted_value}"
            else:
                text = f"Widerstand: {formatted_value} (alt)"
        else:
            text = "Warte auf Daten..."

        # Text rendern und anzeigen
        text_surface = font.render(text, True, white)
        text_rect = text_surface.get_rect(center=(screen_width // 2, screen_height // 2))
        screen.blit(text_surface, text_rect)

        pygame.display.flip()  # Bildschirm aktualisieren
        clock.tick(30)  # Maximal 30 FPS

# Hauptprogramm starten
if __name__ == "__main__":
    main()
