"""
CanSat 3D Terrain Viewer
Reads ultrasonic terrain data from Arduino via Serial and displays
a real-time 3D surface map using matplotlib.

Usage:
    python terrain_viewer.py COM4
    python terrain_viewer.py COM4 --demo  (for testing without Arduino)

Requirements:
    pip install pyserial matplotlib numpy
"""

import sys
import json
import time
import numpy as np
import matplotlib
matplotlib.use('TkAgg')
import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D
from matplotlib import cm
from matplotlib.colors import Normalize
import serial
import threading

# ============== SENSOR POSITIONS ==============
# Layout matches physical arrangement:
#
#    S1 (Top-L) ────── S2 (Top-R)
#         \              /
#          \    S0      /
#           \  (Center)/
#         /              \
#    S3 (Bot-L) ────── S4 (Bot-R)

NUM_SENSORS = 5
SENSOR_LABELS = ['S0\nCenter', 'S1\nTop-L', 'S2\nTop-R', 'S3\nBot-L', 'S4\nBot-R']

# Sensor X,Y positions (center=0,0, corners at ±1)
SENSOR_X = np.array([0.0, -1.0,  1.0, -1.0,  1.0])
SENSOR_Y = np.array([0.0,  1.0,  1.0, -1.0, -1.0])


class TerrainViewer:
    def __init__(self, port=None, demo=False):
        self.demo = demo
        self.port = port
        self.serial_conn = None
        self.terrain_data = {f's{i}': 50 for i in range(NUM_SENSORS)}
        self.terrain_data.update({'min': 50, 'max': 50, 'var': 0, 'status': 'INIT'})
        self.running = True
        self.data_lock = threading.Lock()
        self.scan_count = 0
        
    def connect(self):
        if self.demo:
            print("[DEMO] Running in demo mode - generating fake terrain data")
            return True
        try:
            self.serial_conn = serial.Serial(self.port, 115200, timeout=1)
            print(f"[OK] Connected to {self.port}")
            time.sleep(2)
            return True
        except Exception as e:
            print(f"[ERR] Cannot connect to {self.port}: {e}")
            return False
    
    def read_serial_thread(self):
        while self.running:
            try:
                if self.demo:
                    time.sleep(0.8)
                    base = 50 + 20 * np.sin(time.time() * 0.5)
                    noise = np.random.normal(0, 3, NUM_SENSORS)
                    # Simulate terrain with a slope
                    slope = np.array([0, -12, 8, -8, 15]) * np.sin(time.time() * 0.3)
                    distances = base + noise + slope
                    distances = np.clip(distances, 2, 400)
                    
                    with self.data_lock:
                        for i in range(NUM_SENSORS):
                            self.terrain_data[f's{i}'] = float(distances[i])
                        self.terrain_data['min'] = float(np.min(distances))
                        self.terrain_data['max'] = float(np.max(distances))
                        rng = float(np.max(distances) - np.min(distances))
                        self.terrain_data['var'] = rng
                        if rng > 50:
                            self.terrain_data['status'] = 'HAZARD!'
                        elif rng > 20:
                            self.terrain_data['status'] = 'UNEVEN'
                        else:
                            self.terrain_data['status'] = 'FLAT-SAFE'
                        self.scan_count += 1
                    continue
                
                if self.serial_conn and self.serial_conn.in_waiting:
                    line = self.serial_conn.readline().decode('utf-8', errors='ignore').strip()
                    if line.startswith('TERRAIN:'):
                        json_str = line[8:]
                        try:
                            data = json.loads(json_str)
                            with self.data_lock:
                                self.terrain_data = data
                                self.scan_count += 1
                        except json.JSONDecodeError:
                            pass
            except Exception as e:
                if self.running:
                    time.sleep(1)
    
    def create_surface_grid(self, distances):
        """Create smooth surface from 5 points using IDW interpolation"""
        grid_size = 25
        xi = np.linspace(-1.5, 1.5, grid_size)
        yi = np.linspace(-1.5, 1.5, grid_size)
        XI, YI = np.meshgrid(xi, yi)
        ZI = np.zeros_like(XI)
        
        for i in range(grid_size):
            for j in range(grid_size):
                weights = []
                values = []
                for k in range(NUM_SENSORS):
                    dx = XI[i, j] - SENSOR_X[k]
                    dy = YI[i, j] - SENSOR_Y[k]
                    dist = np.sqrt(dx*dx + dy*dy)
                    if dist < 0.01:
                        ZI[i, j] = distances[k]
                        break
                    w = 1.0 / (dist ** 2.5)
                    weights.append(w)
                    values.append(distances[k])
                else:
                    total_weight = sum(weights)
                    ZI[i, j] = sum(w * v for w, v in zip(weights, values)) / total_weight
        return XI, YI, ZI
    
    def run(self):
        if not self.connect():
            return
        
        serial_thread = threading.Thread(target=self.read_serial_thread, daemon=True)
        serial_thread.start()
        
        # Setup figure with dark theme
        fig = plt.figure(figsize=(14, 8))
        fig.patch.set_facecolor('#1a1a2e')
        
        ax3d = fig.add_subplot(121, projection='3d')
        ax3d.set_facecolor('#16213e')
        ax2d = fig.add_subplot(122)
        ax2d.set_facecolor('#16213e')
        
        plt.subplots_adjust(top=0.88, bottom=0.10, left=0.05, right=0.95, wspace=0.25)
        fig.suptitle('🛰️ CanSat Terrain Scanner (5 Sensors)', fontsize=18, 
                     fontweight='bold', color='#e94560', y=0.96)
        
        print("[OK] 3D Terrain Viewer started! Close the window to stop.")
        
        while self.running:
            try:
                with self.data_lock:
                    data = self.terrain_data.copy()
                    scan = self.scan_count
                
                distances = np.array([max(data.get(f's{i}', 0), 0) for i in range(NUM_SENSORS)])
                status = data.get('status', 'INIT')
                
                status_colors = {
                    'FLAT-SAFE': '#00d474', 'UNEVEN': '#ffa500',
                    'HAZARD!': '#ff0000', 'NO_GROUND': '#ff00ff', 'INIT': '#808080'
                }
                status_color = status_colors.get(status, '#ffffff')
                
                # ===== 3D SURFACE =====
                ax3d.clear()
                ax3d.set_facecolor('#16213e')
                
                if np.any(distances > 0):
                    XI, YI, ZI = self.create_surface_grid(distances)
                    norm = Normalize(vmin=np.min(ZI), vmax=np.max(ZI))
                    colors = cm.RdYlGn(1 - norm(ZI))
                    ax3d.plot_surface(XI, YI, ZI, facecolors=colors, alpha=0.85,
                                     edgecolor='none', shade=True)
                    
                    for k in range(NUM_SENSORS):
                        ax3d.scatter(SENSOR_X[k], SENSOR_Y[k], distances[k],
                                    c='#00ffff', s=100, marker='o', 
                                    edgecolors='white', linewidth=1.5, zorder=5)
                        ax3d.text(SENSOR_X[k], SENSOR_Y[k], distances[k] + 3,
                                 f'{distances[k]:.0f}cm', color='white', fontsize=9,
                                 ha='center', fontweight='bold')
                
                ax3d.set_xlabel('X', color='#a0a0a0', fontsize=9)
                ax3d.set_ylabel('Y', color='#a0a0a0', fontsize=9)
                ax3d.set_zlabel('Distance (cm)', color='#a0a0a0', fontsize=9)
                ax3d.set_title(f'3D Terrain Surface\nScan #{scan}', color='white', fontsize=12)
                ax3d.tick_params(colors='#a0a0a0')
                
                # ===== 2D HEATMAP =====
                ax2d.clear()
                ax2d.set_facecolor('#16213e')
                
                if np.any(distances > 0):
                    XI, YI, ZI = self.create_surface_grid(distances)
                    ax2d.contourf(XI, YI, ZI, levels=15, cmap='RdYlGn_r', alpha=0.9)
                    ax2d.contour(XI, YI, ZI, levels=8, colors='white', linewidths=0.3, alpha=0.5)
                    
                    for k in range(NUM_SENSORS):
                        ax2d.plot(SENSOR_X[k], SENSOR_Y[k], 'wo', markersize=10,
                                 markeredgecolor='black', markeredgewidth=2)
                        ax2d.annotate(f'S{k}\n{distances[k]:.0f}cm',
                                     (SENSOR_X[k], SENSOR_Y[k]),
                                     textcoords="offset points", xytext=(0, 14),
                                     ha='center', color='white', fontsize=9, fontweight='bold',
                                     bbox=dict(boxstyle='round,pad=0.2', facecolor='#333',
                                              edgecolor='none', alpha=0.8))
                    
                    # Draw the sensor box outline
                    box_x = [-1, 1, 1, -1, -1]
                    box_y = [1, 1, -1, -1, 1]
                    ax2d.plot(box_x, box_y, '--', color='#ffffff', linewidth=1, alpha=0.4)
                
                ax2d.set_xlabel('X', color='#a0a0a0')
                ax2d.set_ylabel('Y', color='#a0a0a0')
                ax2d.set_title(f'Top-Down View | Status: {status}',
                              color=status_color, fontsize=13, fontweight='bold')
                ax2d.set_aspect('equal')
                ax2d.tick_params(colors='#a0a0a0')
                
                # Status bar
                rng = data.get('max', 0) - data.get('min', 0)
                info = (f'Min: {data.get("min", 0):.0f}cm | '
                       f'Max: {data.get("max", 0):.0f}cm | '
                       f'Range: {rng:.0f}cm | Status: {status}')
                
                # Clear old texts and redraw
                while len(fig.texts) > 1:
                    fig.texts[-1].remove()
                fig.text(0.5, 0.03, info, ha='center', fontsize=12,
                        color=status_color, fontweight='bold',
                        bbox=dict(boxstyle='round', facecolor='#0f3460',
                                 edgecolor=status_color, alpha=0.9))
                
                plt.pause(0.5)
                
            except KeyboardInterrupt:
                break
            except Exception as e:
                print(f"[ERR] {e}")
                time.sleep(0.5)
        
        self.running = False
        if self.serial_conn:
            self.serial_conn.close()
        plt.close()


def main():
    print("=" * 50)
    print("  CanSat 3D Terrain Viewer v1.0 (5 Sensors)")
    print("=" * 50)
    
    demo = '--demo' in sys.argv
    
    if demo:
        port = None
    elif len(sys.argv) < 2:
        print("\nUsage: python terrain_viewer.py COM4")
        print("       python terrain_viewer.py --demo")
        print("\nAvailable ports:")
        try:
            import serial.tools.list_ports
            for p in serial.tools.list_ports.comports():
                print(f"  {p.device} - {p.description}")
        except:
            print("  (install pyserial to list ports)")
        return
    else:
        port = sys.argv[1]
    
    viewer = TerrainViewer(port=port, demo=demo)
    viewer.run()


if __name__ == '__main__':
    main()
