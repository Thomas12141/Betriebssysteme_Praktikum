<div align="center">

# Betriebssysteme

</div>

## Inhalt

- [Betriebssysteme](#betriebssysteme)
  - [Inhalt](#inhalt)
  - [Getting Started](#getting-started)
    - [Prerequisites](#prerequisites)
    - [Setup](#setup)
      - [Clonen des Projektes](#clonen-des-projektes)
    - [Building](#building)
    - [Hinzufügen von osmp\_executables](#hinzufügen-von-osmp_executables)
    - [Testing](#testing)
      - [Aufbau der test.json](#aufbau-der-testjson)
    - [CI/CD Testing](#cicd-testing)
  - [Overview ](#overview-)

## Getting Started

Auf unseren Laborrechnern ist aktuell Ubuntu LTS in der aktuellsten Version (Stand: 19.03.2024) installiert.
Wir erwarten, dass Ihr Praktikumsprojekt auf diesen Rechnern fehlerfrei kompiliert werden kann und zum Abschluss fehlerfrei funktioniert. 
Mit dieser Anleitung möchten wir Sie unterstützen, dass Sie Ihr Praktikumsprojekt auch auf Ihrer eigenen Ubuntu-Installation weiterentwickeln können. 
Im Praktikum empfehlen wir Ihnen, ausschließlich die Laborrechner zum Entwickeln der Lösung zu nutzen.

Unsere Laborrechner sind für das Praktikum vorbereitet und enthalten insbesondere von uns geforderte Programme.
Zu Beginn sind jedoch einige persönliche Einstellungen noch zu erledigen, die unter dem Punkt [Setup](#setup) besprochen werden.

>**Anmerkung:** \
> Obwohl Sie sich mit Ihrer FH-Kennung auch an unseren Rechnern mit dem gewohnten Passwort anmelden können, verwenden wir eigene Homeverzeichnisse, die von eigenen Servern gemountet werden.
> 
> Unser Labornetz ist zudem von außen **nicht** ohne Weiteres nutzbar.
> Obwohl es technisch möglich wäre, über einen Verbindungsrechner auf die Laborrechner zuzugreifen, können wir die Rechner außerhalb der offiziellen Praktikumszeiten nicht ohne Weiteres zur Verfügung stellen.
> Sorgen Sie also bitte dafür, dass Sie Ihre Dateien zum Ende des Praktikums möglichst in Ihrem Git-Repository ablegen.

### Prerequisites

Um dieses Projekt lokal zu nutzen, wird Git benötigt, da es uns ermöglicht, den gesamten Projektcode von GitLab zu klonen/kopieren und Änderungen mit dem Stand im GitLab zu synchronisieren.

```sh
sudo apt-get install git
```

Um das Praktikumsprojekt benutzen zu können, müssen Sie cmake und gcc installiert haben.

```sh
sudo apt-get install cmake gcc
```

Damit Sie das docs/generate_Documentation.sh Skript verwenden können, um die doxygen-Dokumentation zu erstellen, benötigen Sie doxygen, make, pdflatex und graphviz.

```sh
sudo apt-get install doxygen make texlive-latex-base graphviz
```

### Setup

Die Versionierung des Praktikums geschieht über das GitLab der FH-Münster. Um lokal auf einem Rechner an dem Projekt weiterzuentwickeln, muss das Projekt lokal auf den Rechner kopiert werden.

Hilfreich, um git/gitlab kennenzulernen: 

- [https://git.fh-muenster.de/help/topics/git/get_started.md](https://git.fh-muenster.de/help/topics/git/get_started.md)
- [https://about.gitlab.com/images/press/git-cheat-sheet.pdf](https://about.gitlab.com/images/press/git-cheat-sheet.pdf)

SSH-Key für die Kommunikation mit dem Gitlab aufsetzen: 

- [https://docs.gitlab.com/ee/user/ssh.html](https://docs.gitlab.com/ee/user/ssh.html)

#### Clonen des Projektes

```sh
cd my-folder
git clone ssh://git@git.fh-muenster.de:2323/<link-zum-projekt>.git
cd <projektordner>
```

### Building

Das Projekt lässt sich wie folgt per CMake bauen

CMake: [https://cmake.org/getting-started/]

```sh
cd /path/to/project

cmake . -B ./cmake-build-debug

cd cmake-build-debug

cmake --build .
```

### Hinzufügen von osmp_executables

Das Hinzufügen von osmp_executables wird folgend am Beispiel einer osmpExecutable_echoall.c demonstriert.

Um eine weitere Executable "osmpExecutable_echoall" zum Bausystem hinzuzufügen, müssen folgende Einträge vorgenommen werden:

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

Sie können Test Cases in der test/tests.json definieren.

Über die folgenden Skripte können diese Tests ausgeführt werden.

- **runAllTests.sh** führt alle Tests, die in der tests.json definiert sind, aus.
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

#### Aufbau der test.json

Die *test.json* beinhaltet eine Liste von Ausführungen der verschiedenen osmp_executables und lässt sich beliebig erweitern.

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

Die Parameter der Ausführung werden in den entsprechenden Variablen angegeben; "TestName" ist frei wählbar und dient nur der Zuordnung.
Der Testname sollte **nicht** mehrfach vorkommen. Sonst führt das runOneTest.sh Skript diesen nicht aus.

Ist **PfadZurLogDatei** ein leerer String (""), wird dieser nicht verwendet und auch kein -L als Argument übergeben.
Es kann jedoch ein Leerzeichen als Pfad angegeben werden (" "). In diesem Fall wird lediglich ein -L als Argument übergeben.

Die **LogVerbositaet** wird nur dann nicht übergeben, wenn diese auf 0 gesetzt wird.
Auch hier kann beim Angeben eines Leerzeichens lediglich das "-V" als Argument getestet werden.

>**NOTE:** Ein Test wird als "Passed" angesehen, falls der OSMP-Starter mit dem exitCode 0 beendet wird (Um z. B. Synchronisation zu testen reichen diese tests nicht aus).

### CI/CD Testing

Sobald sie ihren Code zu GitLab pushen, wird automatisch eine Pipeline gestartet, die alle Tests ausführt und auf Ihren Erfolg prüft.

## Overview <a name="overview"></a>

Das Projekt kommt mit ein paar Beispiel OSMP-Executables, der aus zu implementierenden Header Datei der OSMP-Library, als auch dem grundlegenden OSMP-Runner

```text
📁<projektordner>
├── 📁docs/
│   ├── Doxyfile
│   ├── generate_Documentation.sh
│   └── OSMP_Documentation.pdf
├── 📁src/
│   ├── 📁osmp_executables/
│   │   ├── osmp_Executable_SendIRecv.c
│   │   └── osmp_Executable_SendRecv.c
│   ├── 📁osmp_library/
│   │   ├── OSMP.h
│   │   ├── osmplib.c
│   │   └── osmplib.h
│   └── 📁osmp_runner/
│       ├── osmp_run.c
│       └── osmp_run.h
├── 📁test/
│   ├── common.sh
│   ├── runAllTests.sh
│   ├── runAllTestsForOneExecutable.sh
│   ├── runOneTest.sh
│   └── tests.json
├── .gitignore
├── .gitlab-ci.yml
├── CMakeLists.txt
└── README.md
```

<div align="right" style="text-align: right">

   [(nach oben)](#betriebssysteme)

</div>