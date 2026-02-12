import json
import sys

if len(sys.argv) < 2:
    print('Использование: python extract_svg.py <input.json>')
    sys.exit(1)

input_file = sys.argv[1]

try:
    with open(input_file, 'r', encoding='utf-8') as f:
        data = json.load(f)
except FileNotFoundError:
    print(f'Ошибка: файл {input_file} не найден')
    sys.exit(1)
except json.JSONDecodeError:
    print(f'Ошибка: файл {input_file} не является корректным JSON')
    sys.exit(1)

for item in data:
    if 'map' in item:          # ✅ правильная проверка
        svg = item['map']
        output_file = input_file.replace('.json', '.svg')  # output.json → output.svg
        with open(output_file, 'w', encoding='utf-8') as out:
            out.write(svg)
        print(f'SVG сохранён в {output_file}')
        break
else:
    print('Объект с картой (поле "map") не найден')
    sys.exit(1)