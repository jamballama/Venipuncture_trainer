import serial
import pygame
import sys
import time

# -------------------
# Einstellungen
# -------------------
serial_port = "COM12"
baud_rate = 115200  # Baudrate des Arduino

# Fenstergröße
screen_width, screen_height = 1280, 800

# Grafikeinstellungen
pygame.init()
screen = pygame.display.set_mode((screen_width, screen_height))
pygame.display.set_caption("Widerstandsanzeige")

# Schriften
font_large = pygame.font.Font(None, 90)   # für den großen Anzeigetext
font_small = pygame.font.Font(None, 30)   # für Achsenbeschriftungen/Labels

# Farben
black = (0, 0, 0)
white = (255, 255, 255)
red   = (255,   0,   0)

# Variablen
last_resistance_value = None
last_update_time = time.time()
data_points = []  # (timestamp, resistance)

# Schwellwert, unter dem der Bereich rot markiert werden soll
ALERT_THRESHOLD = 1200.0  # Ohm

def format_resistance_fixed(value, width=12):
    """
    Formatiert den Widerstandswert mit fester Gesamtbreite,
    damit bei Wechsel zwischen 'Ohm', 'kOhm', 'MOhm' der Text nicht springt.
    width bestimmt die Breite des generierten Strings (inkl. Einheit).
    """
    if value < 1000:
        s = f"{value:.2f} Ohm"
    elif value < 1e6:
        s = f"{value/1000:.1f} kOhm"
    else:
        s = f"{value/1e6:.1f} MOhm"
    return f"{s:>{width}}"

def main():
    global last_resistance_value, last_update_time, data_points

    # Seriellen Port öffnen
    ser = serial.Serial(serial_port, baud_rate, timeout=1)
    clock = pygame.time.Clock()
    
    # Obergrenze für die Anzeige (Clamping)
    max_display_value = 1e7  # 10 MOhm, nach Bedarf ändern

    # Abmessungen des Diagramms
    x_margin = 80
    y_margin = 80
    graph_width = screen_width - 2 * x_margin
    graph_height = 450
    graph_top = y_margin
    graph_bottom = graph_top + graph_height

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
                last_resistance_value = float(line)
                last_update_time = time.time()
        except ValueError:
            # Ungültige Werte ignorieren
            pass

        current_time = time.time()

        # Datenpunkt speichern
        if last_resistance_value is not None:
            data_points.append((current_time, last_resistance_value))
            # Nur die letzten 30 Sekunden behalten
            data_points = [dp for dp in data_points if current_time - dp[0] <= 30]

        # Bildschirm löschen
        screen.fill(black)

        # -------------------
        # Diagramm zeichnen
        # -------------------
        pygame.draw.rect(screen, white, (x_margin, y_margin, graph_width, graph_height), 1)

        if data_points:
            values = [v for (_, v) in data_points]
            min_value = min(values)
            max_value = max(values)

            # Clamping
            min_value = max(min_value, 0.0)
            max_value = min(max_value, max_display_value)

            if min_value == max_value:
                min_value -= 1
                max_value += 1

            oldest_time = current_time - 30

            #
            # --- Roten Bereich für Werte unter ALERT_THRESHOLD füllen ---
            #
            if ALERT_THRESHOLD < min_value:
                # Alles liegt über ALERT_THRESHOLD -> kein roter Bereich
                pass
            elif ALERT_THRESHOLD > max_value:
                # Alles liegt unter ALERT_THRESHOLD -> kompletten Graph rot füllen
                pygame.draw.rect(screen, red, (x_margin, graph_top, graph_width, graph_height))
            else:
                # Teilbereich füllen: von ALERT_THRESHOLD bis zum unteren Rand
                # 1) Y-Koordinate für ALERT_THRESHOLD berechnen
                threshold_y = graph_bottom - ((ALERT_THRESHOLD - min_value) / (max_value - min_value)) * graph_height
                # Clampen, damit wir nicht außerhalb des Diagramms zeichnen
                if threshold_y < graph_top:
                    threshold_y = graph_top
                if threshold_y > graph_bottom:
                    threshold_y = graph_bottom

                fill_height = graph_bottom - threshold_y
                if fill_height > 0:
                    pygame.draw.rect(screen, red, (x_margin, threshold_y, graph_width, fill_height))

            #
            # --- Nun die Datenpunkte als weiße Linie zeichnen ---
            #
            points = []
            for (t, val) in data_points:
                # X-Koordinate
                x = x_margin + ((t - oldest_time) / 30.0) * graph_width
                # Y-Koordinate (Widerstand linear skaliert)
                val_clamped = min(val, max_display_value)
                y = graph_bottom - ((val_clamped - min_value) / (max_value - min_value)) * graph_height
                points.append((x, y))

            if len(points) >= 2:
                pygame.draw.lines(screen, white, False, points, 2)

            #
            # --- Y-Achsenbeschriftung ---
            #
            min_label_str = format_resistance_fixed(min_value, 8)
            max_label_str = format_resistance_fixed(max_value, 8)

            min_label = font_small.render(min_label_str, True, white)
            max_label = font_small.render(max_label_str, True, white)

            # minimaler Wert
            screen.blit(min_label, (x_margin + 10,
                                    graph_bottom - min_label.get_height() / 2))
            # maximaler Wert
            screen.blit(max_label, (x_margin + 10,
                                    graph_top - max_label.get_height() / 2))

            # Y-Achsen-Label
            y_axis_label = font_small.render("Resistance", True, white)
            screen.blit(y_axis_label, (x_margin + 10, graph_top + 10))

            #
            # --- X-Achsenbeschriftung (Zeit) ---
            #
            for sec_mark in [0, 10, 20, 30]:
                x_pos = x_margin + (sec_mark / 30.0) * graph_width
                mark_label = font_small.render(str(sec_mark), True, white)
                screen.blit(mark_label, (x_pos - mark_label.get_width() / 2,
                                         graph_bottom + 5))

            # X-Achsen-Label
            x_axis_label = font_small.render("Time (s)", True, white)
            screen.blit(x_axis_label, (x_margin + graph_width - x_axis_label.get_width(),
                                       graph_bottom + 30))

        # -------------------
        # Textanzeige (unten)
        # -------------------
        if last_resistance_value is not None:
            val_str = format_resistance_fixed(last_resistance_value, 8)
            if current_time - last_update_time <= 1:
                text_str = f"Widerstand: {val_str}"
            else:
                text_str = f"Widerstand: {val_str} (alt)"
        else:
            text_str = "Warte auf Daten..."

        text_surface = font_large.render(text_str, True, white)
        text_x = x_margin
        text_y = screen_height - 120
        screen.blit(text_surface, (text_x, text_y))

        pygame.display.flip()
        clock.tick(30)  # Maximal 30 FPS

if __name__ == "__main__":
    main()
