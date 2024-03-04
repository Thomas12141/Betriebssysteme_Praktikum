<a name="readme-top"></a>

<div align="center">

  <h><b>Betriebssysteme</b></h>

</div>

# Inhalt

- [Getting Started](#getting-started)
  - [Prerequisites](#prerequisites)
  - [Setup](#setup)
  - [Building](#Building)
  - [Adding Executables](#osmp_executables-hinzufügen)
  - [Run tests](#run-tests)
  - [CI/CD Testing](#CI/CD-Testing)

<!-- GETTING STARTED -->

## Getting Started <a name="getting-started"></a>

### Prerequisites

```sh
 cmake
 gcc
```

### Setup

```sh
# Clonen des Projektes
cd my-folder
git clone ssh://git@git.fh-muenster.de:2323/<link-zum-projekt>/bs-labor-test.git
```

### Building

```sh
cmake . -B ./cmake-build-debug

cd cmake-build-debug

cmake --build .
```

### osmp_executables hinzufügen

Um eine weitere Executable "echoall" zum Baussystem hinzuzufügen müssen folgende einträge vorgenommen werden:
```sh
# CMakeLists.txt
set(SOURCES_FOR_EXECUTABLE_ECHOALL # <- NAME der Executable innerhalb von CMake
        src/executables/osmpExecutable_echoall.c src/executables/osmpExecutable_echoall.h # <- Source und Header Datien für de Executable
        ${MAIN_SOURCES_FOR_EXECUTABLES}) # <- Bereits besetzte variable mit anderen dateien, z.B. OSMP.h

<!-- -->

add_executable(osmpExecutable_echoall ${SOURCES_FOR_EXECUTABLE_ECHOALL} ) # <- Executable bauen lassen

<!-- -->

target_link_libraries(osmpExecutable_echoall ${LIBRARIES}) # <- Genutzte Bibliotheken linken
```

### Testing


```sh
# Test einer osmp_executable "echoall"
./osmp_run 5 -L /mylog.log -V 3 ./echoall 1234 param2

# der Test ist erfolgreich wenn das Programm sich erfolgreich, ohne speicherfehler oder dergleichen beendet
```

Das einfache durchlaufen beliebiger osmp_executable reicht aber of nicht um die funktionalität eines programmes zu verifizieren.

Um alle Tests auszuführen:

```sh
  ./test/runAllTests.sh
```

### CI/CD Testing

Wenn dieses Repository in Gitlab gepusht wird wird automatische eine pipeline gestartet die alle Test ausführt und auf ihren erfolg prüft.

<p align="right">(<a href="#readme-top">nach oben</a>)</p>