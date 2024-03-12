<a name="readme-top"></a>

<div align="center">
   <h1>
      <b>Betriebssysteme</b>
   </h1>
</div>

## Inhalt

- [Inhalt](#inhalt)
- [Getting Started ](#getting-started-)
  - [Prerequisites](#prerequisites)
  - [Setup](#setup)
  - [Building](#building)
  - [Hinzufügen von osmp\_executables](#hinzufügen-von-osmp_executables)
  - [Testing](#testing)
    - [Aufbau test.json](#aufbau-testjson)
  - [CI/CD Testing](#cicd-testing)
- [Overview ](#overview-)

## Getting Started <a name="getting-started"></a>

Das Praktikum-Projekt hat den Anspruch daran auf Ubuntu zu laufen, insbesondere auf den Laborrechnern. Die Anleitung hilft gewissermaßen auch dabei das Projekt auf einer eigenen Ubuntu installation zum laufen zu bekommen. Wir empfehlen ausschließlich auf den Laborrechner zu arbeiten, ggf. von Zuhause über SSH (Eine Anleitung dafür wird zu Verfügung gestellt). Wenn Sie auf einem der Laborrechner arbeiten, müssen Sie nichts installieren und können direkt zum Punkt [Setup](#setup) übergehen.

### Prerequisites

Um das Praktikums-Projekt benutzen zu können, müssen Sie cmake und gcc installiert haben.
Für die Dokumentation benötigen Sie doxygen, make, pdflatex und graphviz.

```sh
sudo apt-get install cmake gcc doxygen make texlive-latex-base grpahviz
```

### Setup

Die Versionierung des Praktikums geschieht über das Gitlab der FH-Münster. Um lokal auf einem Rechner an dem Projekt weiterzuentwickeln muss das Projekt lokal auf den Rechner kopiert werden.

SSH-Key für die Kommunikation mit dem Gitlab aufsetzen: [https://docs.gitlab.com/ee/user/ssh.html]
Eventuell hilfreich um git/gitlab kennenzulernen: [https://git.fh-muenster.de/help/topics/git/get_started.md], [https://about.gitlab.com/images/press/git-cheat-sheet.pdf]

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
cd /projectdir

cmake . -B ./cmake-build-debug

cd cmake-build-debug

cmake --build .
```

### Hinzufügen von osmp_executables

Das Hinzufügen von osmp_executables wird folgend am Beispiel einer osmpExecutable_echoall.c demonstriert.

Um eine weitere Executable "osmpExecutable_echoall" zum Bausystem hinzuzufügen müssen folgende Einträge vorgenommen werden:

```sh
# ./CMakeLists.txt
...

set(SOURCES_FOR_EXECUTABLE_ECHOALL # <- NAME der Executable innerhalb von CMake
    src/executables/osmpExecutable_echoall.c src/executables/osmpExecutable_echoall.h # <- Source und Header Dateien für die Executable
    ${MAIN_SOURCES_FOR_EXECUTABLES} # <- Bereits besetzte Variable mit anderen Dateien, z. B. OSMP.h
) 

...

add_executable(osmpExecutable_echoall ${SOURCES_FOR_EXECUTABLE_ECHOALL} ) # <- Executable bauen lassen

...

target_link_libraries(osmpExecutable_echoall ${LIBRARIES}) # <- Genutzte Bibliotheken linken
```

### Testing

Sie können TestCases in der test/tests.json definieren.

Über die folgenden Skripte können diese Tests ausgeführt werden.

- **runAllTests.sh** führt alle Tests, die in der tests.json definiert sind aus.
  ```sh
  # Usage
  ./runAllTests.sh
  ```
- **runAllTestsForOneExecutable.sh** erwartet einen Executable-Namen und führt alle Tests aus, die diese Executable nutzen.
  ```sh
  # Usage
  ./runAllTestsForOneExecutable.sh osmpExecutable_exampleExecutable
  ```
- **runOneTest.sh** erwartet einen Testnamen und führt diesen Test aus.
  ```sh
  # Usage
  ./runOneTest.sh ExampleTest
  ```

#### Aufbau test.json

Die *test.json* beinhaltet eine Reihe von Ausführungen der verschiedenen osmp_executables und lässt sich beliebig erweitern.

```json
{
   "TestName": "ExampleTest",
   "ProcAnzahl": 5,
   "PfadZurLogDatei": "/path/to/logfile",
   "LogVerbositaet": 5,
   "osmp_executable": "osmpExecutable_exampleExecutable",
   "parameter": [
      "param1",
      "param2",
      7,
      "param4"
   ]
}
```

Die Parameter der Ausführung werden in den entsprechenden Variablen Angegeben, "TestName" is frei wählbar und dient nur der Zuordnung.
Der Test Name sollte **nicht** mehrfach vorkommen. Sonst führt das runOneTest.sh Skript diesen nicht aus.

Ist **PfadZurLogDatei** eine Leerer String (""), wird dieser nicht verwendet und auch kein -L als Argument übergeben.
Es kann jedoch ein Leerzeichen als Pfad angegeben werden (" "). In diesem Fall wird lediglich ein -L als Argument übergeben.

Die **LogVerbositaet** wird nur dann nicht übergeben, wenn diese auf 0 gesetzt wird.
Auch hier kann bei angeben eines Leerzeichens lediglich das "-V" als Argument getestet werden.

>**NOTE:** Ein Test wird als "Passed" angesehen, falls der OSMP-Starter mit dem exitCode 0 beendet wird (Um z. B. Synchronisation zu testen reichen diese tests nicht aus).

### CI/CD Testing

Wenn dieses Repository in Gitlab gepusht wird wird automatische eine pipeline gestartet die alle Test ausführt und auf ihren Erfolg prüft.

## Overview <a name="overview"></a>

Das Projekt kommt mit ein paar beispiel OSMP-Executables, der aus zu implementierenden Header Datei der OSMP-Library, als auch dem grundlegendem OSMP-Runner

- src/
   - osmp_executables/
      - osmpExecutable_SendIRecv.c
      - osmpExecutable_SendRecv.c
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

[def]: #inhalt