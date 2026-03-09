import pymeshlab as ml
import os

# Шлях до папки зі скриптом
current_dir = os.path.dirname(os.path.abspath(__file__))

# Налаштування відсотків
standard_percentage = 0.2  # 20% для ніг
ball_percentage = 0.01     # 1% для контактних кульок (ступень)

# Список файлів, які НЕ чіпаємо взагалі
skip_files = ['base_link.stl', 'cover.stl']

def simplify_folder():
    ms = ml.MeshSet()
    
    files = [f for f in os.listdir(current_dir) if f.lower().endswith('.stl')]
    
    if not files:
        print("❌ STL файлів не знайдено.")
        return

    print(f"--- Починаю обробку ({len(files)} файлів) ---")

    for filename in files:
        fname_lower = filename.lower()
        
        # 1. Перевірка на скіп
        if fname_lower in [f.lower() for f in skip_files]:
            print(f"⏩ Скіп: {filename} (корпус)")
            continue

        file_path = os.path.join(current_dir, filename)
        
        # 2. Визначаємо рівень спрощення
        if "tip" in fname_lower:
            target = ball_percentage
            mode_label = "КУЛЬКА (5%)"
        else:
            target = standard_percentage
            mode_label = "НОГА (20%)"

        try:
            ms.load_new_mesh(file_path)
            initial_faces = ms.current_mesh().face_number()
            
            # Застосовуємо фільтр
            ms.apply_filter('meshing_decimation_quadric_edge_collapse', 
                            targetperc=target,
                            preservenormal=True,
                            preservetopology=True)

            ms.save_current_mesh(file_path)
            
            new_faces = ms.current_mesh().face_number()
            print(f"✅ {filename}: {initial_faces} -> {new_faces} граней [{mode_label}]")
            
            ms.clear()
            
        except Exception as e:
            print(f"❌ Помилка у {filename}: {str(e)}")

if __name__ == "__main__":
    simplify_folder()
    print("--- Обробку завершено! Перевірте робота у Webots. ---")
