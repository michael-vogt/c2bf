# C-to-Brainfuck Konverter (c2bf)

## Übersicht

Dieser Konverter übersetzt ein eingeschränktes C-Subset in Brainfuck-Code.

Das Ziel ist es, einfache C-Programme in eine minimale, esoterische Zielsprache zu transformieren.

---

## Unterstütztes C-Subset

### Typen
- `int` (einziger unterstützter Datentyp)

### Variablen
- Deklaration von Variablen
- Zuweisungen

### Arithmetik
- Addition `+`
- Subtraktion `-`
- Multiplikation `*`
- Division `/`

### Vergleiche
- `==`
- `!=`
- `<`
- `>`
- `<=`
- `>=`

### Kontrollfluss
- `if`
- `else`
- `while`

### Ein-/Ausgabe
- `putchar()`
- `getchar()`

### Funktionen
- ausschließlich `main()`

---

## Eingabequellen für C-Code

Der zu konvertierende C-Code kann auf drei Arten übergeben werden:

### 1. Kommandozeilen-Argument (Dateipfad)

Der Pfad zu einer Datei wird als Argument übergeben:

```bash
./c2bf.exe program.c
````

---

### 2. Pipe über `stdin`

Der C-Code wird über die Standard-Eingabe übergeben:

```bash
cat program.c | ./c2bf.exe
```

oder:

```bash
echo "int main() { return 0; }" | ./c2bf.exe
```

---

### 3. Interaktive Eingabe (Fallback)

Falls weder Datei noch Pipe verwendet wird, kann der Code direkt im Programm eingegeben werden:

```bash
./c2bf.exe
```

Beenden der Eingabe erfolgt über:

* **Linux/macOS:** `Ctrl + D`
* **Windows:** `Enter`, `Ctrl + Z`, `Enter`

---

## Verhalten des Programms

Das Programm entscheidet automatisch:

1. Wenn ein Dateipfad als Argument übergeben wird → Datei wird geladen
2. Wenn Daten über `stdin` gepiped werden → Eingabe wird daraus gelesen
3. Andernfalls → interaktive Eingabe über Terminal

---

## Beispiel

```c
int main() {
    int x = 10;
    putchar(x);
    return 0;
}
```

---

## Ziel

Der Konverter übersetzt das C-Subset in äquivalenten Brainfuck-Code.

```
```
