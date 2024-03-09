<a name="readme-top"></a>

<div align="center">

  <div align="center"><h><b>Betriebssysteme</b></h></div>

</div>

# Inhalt

- [Inhalt](#inhalt)
  - [Getting Started ](#getting-started-)
    - [Prerequisites](#prerequisites)
    - [Setup](#setup)
    - [Building](#building)
    - [osmp\_executables hinzufügen](#osmp_executables-hinzufügen)
    - [Testing](#testing)
      - [Aufbau test.json](#aufbau-testjson)
    - [CI/CD Testing](#cicd-testing)
  - [Overview ](#overview-)

## Getting Started <a name="getting-started"></a>

Das Praktikum-Projekt hat den anspruch daran auf Ubuntu zu laufen, insbesondere auf den Laborrechnern. Die Anleitung hilft gewissermaßen auch dabei das Projet auf einer eigenen Ubuntu installation zum laufen zu bekommen. Wir empfiehlen allerdings möglichtst auschließlich auf den Laborrechner zu arbeiten, ggf. Zuhause aus per SSH (Eine Anleitung dafür wird zu verfügung gestellt). Wenn du auf den Laborrechner arbeitest, brauchst du nichts installieren und kannst direkt zum Punkt [Setup](#setup) übergehen.

### Prerequisites

Um das Praktikums-Projekt benutzen zu können, musst du cmake und gcc installieren.
Für die Dokumentation benötigst Du doxygen, make, pdflatex und graphviz.

```sh
sudo apt-get install cmake gcc doxygen make texlive-latex-base grpahviz
```

### Setup

Die Versionierung des Praktikums geschieht über das Gitlab Der FH-Münster. Um lokal auf einem Rechner an dem Projekt weiterzuentwickeln muss das Projekt lokal auf den Rechner kopiert werden.

SSH-Key für die Kommunikation mit dem Gitlab aufsetzen: [https://docs.gitlab.com/ee/user/ssh.html]
Eventuell hilfreich um git/gitlab kennzulernen: [https://git.fh-muenster.de/help/topics/git/get_started.md], [https://about.gitlab.com/images/press/git-cheat-sheet.pdf]

```sh
# Clonen des Projektes
cd my-folder
git clone ssh://git@git.fh-muenster.de:2323/<link-zum-projekt>.git
cd <projektordner>
```

### Building

Das Projekt lässt sich wie folgt per CMake bauen

CMake: [https://cmake.org/getting-started/]

```sh
cmake . -B ./cmake-build-debug

cd cmake-build-debug

cmake --build .
```

### osmp_executables hinzufügen

Um eine weitere Executable "echoall" zum Baussystem hinzuzufügen müssen folgende einträge vorgenommen werden:

```sh
# ./CMakeLists.txt
...

set(SOURCES_FOR_EXECUTABLE_ECHOALL # <- NAME der Executable innerhalb von CMake
    src/executables/osmpExecutable_echoall.c src/executables/osmpExecutable_echoall.h # <- Source und Header Datien für de Executable
    ${MAIN_SOURCES_FOR_EXECUTABLES} # <- Bereits besetzte variable mit anderen dateien, z.B. OSMP.h
) 

...

add_executable(osmpExecutable_echoall ${SOURCES_FOR_EXECUTABLE_ECHOALL} ) # <- Executable bauen lassen

...

target_link_libraries(osmpExecutable_echoall ${LIBRARIES}) # <- Genutzte Bibliotheken linken
```

### Testing

Sie können TestCases in der test/tests.json definieren.

#### Aufbau test.json

Die *test.json* beinhaltet eine Reihe von Ausführungen der verschiedenen osmp_executables und lässt sich beliebig erweitern.

```json
{
   "TestName": "ExampleTest",
   "ProcAnzahl": 5,
   "PfadZurLogDatei": "/path/to/logfile",
   "LogVerbositaet": 5,
   "osmp_executable": "exampleExecutable",
   "parameter": [
      "param1",
      "param2",
      7,
      "param4"
   ]
}
```

Die Parameter der Ausführung werden in den entsprechenden Variablen Angegeben, "TestName" is frei wählbar und dient nur der Zuordnung

**NOTE:** Die Variable **TestName** muss eindeutig sein, sonst werden dies

>**NOTE:** Ein Test wird als "Passed" angesehen, falls der OSMP-Starter mit dem exitCode 0 beendet wird (Um z. B. Synchronisation reichen diese tests nicht aus).

### CI/CD Testing

Wenn dieses Repository in Gitlab gepusht wird wird automatische eine pipeline gestartet die alle Test ausführt und auf ihren erfolg prüft.

## Overview <a name="overview"></a>

Das Projekt kommt mit ein paar beispiel OSMP-Executables, der auszuimplementierend Header Datei der OSMP-Library, als auch dem grundlegendem OSMP-Runner

- src/
   - osmp_executables/
      - osmp_Bcast.c
      - osmp_SendIrecv.c
      - osmp_SendRecv.c
   - osmp_library/
      - OSMP.h
      - osmplib.c
      - osmplib.h
   - osmp_runner/
      - osmp_run.c
      - osmp_run.h
        desweiteren gibt es eine Besipielhafte CMakeList.txt und gitlab-ci.yml
- CMakeLists.txt
- .gitlab-ci.yml

<p align="right">(<a href="#readme-top">nach oben</a>)</p>