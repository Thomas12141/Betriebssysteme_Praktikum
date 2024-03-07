/******************************************************************************
* FILE: osmp_Bcast.c
* DESCRIPTION:
* Beinhaltet alle in der Anleitung angegeben Prototypen der OSMP Kernfunktionen wie z.B. OSMP_Test() und vorgegebene Konstanten.
*
* LAST MODIFICATION: March 07, 2024
******************************************************************************/
#ifndef BETRIEBSSYSTEME_OSMP_H
#define BETRIEBSSYSTEME_OSMP_H

#include <stdbool.h>
#include <stddef.h>

/**
 * Alle OSMP-Funktionen liefern im Erfolgsfall OSMP_SUCCESS als Rückgabewert. 
 * Weitere Rückgabewerte können mit Begründung (und Dokumentation!) definiert werden
 */
#define OSMP_SUCCESS 0

/**
 * Im Fehlerfall liefern die OSMP-Funktionen den Wert OSMP_FAILURE zurück. 
 * Die Fehler führen aber nicht zum beenden des Programms (z. B. wenn ein Prozess eine Nachricht an einen nicht existierenden Prozess schickt).
 */
#define OSMP_FAILURE 1

/**
 * Im Fehlerfall liefern die OSMP-Funktionen OSMP_CRITICAL_FAILURE zurück. 
 * Die Fehler sollten zum beenden des Programms führen (z. B. )
 */
#define OSMP_CRITICAL_FAILURE 2

typedef void* OSMP_Request;

/* maximale Zahl der Nachrichten pro Prozess */
#define OSMP_MAX_MESSAGES_PROC 16
/* maximale Anzahl der Nachrichten, die insgesamt vorhanden sein dürfen */
#define OSMP_MAX_SLOTS 256
/* maximale Länge der Nutzlast einer Nachricht */
#define OSMP_MAX_PAYLOAD_LENGTH 1024

/**
 * Die OSMP-Datentypen entsprechen den C-Datentypen. 
 * Sie werden verwendet, um den Typ der Daten anzugeben, die mit den OSMP-Funktionen gesendet bzw. empfangen werden sollen.
 * Mindestens folgende Datentypen *müssen* unterstützt werden:
 */
typedef enum enum_OSMP_Datatype {
    OSMP_SHORT,         // short int
    OSMP_INT,           // int
    OSMP_LONG,          // long int
    OSMP_UNSIGNED_CHAR, // unsigned char
    OSMP_UNSIGNED,      // unsigned
    OSMP_UNSIGNED_SHORT,// unsigned short int
    OSMP_UNSIGNED_LONG, // unsigned long int
    OSMP_FLOAT,         // float
    OSMP_DOUBLE,        // double
    OSMP_BYTE           // char
} OSMP_Datatype;

/**
 * Gibt die maximale Länge der Nutzlast einer Nachricht zurück.
 */
int get_OSMP_MAX_PAYLOAD_LENGTH() {
  return OSMP_MAX_PAYLOAD_LENGTH;
}

/**
 * Gibt die Maximale Anzahl der Nachrichten, die insgesamt vorhanden sein dürfen zurück. 
 */
int get_OSMP_MAX_SLOTS() {
    return OSMP_MAX_SLOTS;
}

/**
 * Gibt die maximale Zahl der Nachrichten pro Prozess zurück.
 */
int get_OSMP_MAX_MESSAGES_PROC() {
    return OSMP_MAX_MESSAGES_PROC;
}

/**
 * Gibt den Wert von OSMP_CRITICAL_FAILURE zurück.
 */
int get_OSMP_CRITICAL_FAILURE() {
    return OSMP_CRITICAL_FAILURE;
}

/**
 * Gibt den Wert von OSMP_FAILURE zurück.
 */
int get_OSMP_FAILURE() {
    return OSMP_FAILURE;
}

/**
 * Gibt den Wert von OSMP_SUCCESS zurück.
 */
int get_OSMP_SUCCESS() {
    return OSMP_SUCCESS;
}

/**
 * Die Funktion OSMP_sizeof() liefert die Größe des Datentyps datatype in Byte zurück.
 *
 * @param [in] datatype OSMP-Datentyp
 *
 * @return Größe des Datentyps in Byte
 */
size_t OSMP_sizeof(OSMP_Datatype datatype);

/**
 * Die Funktion OSMP_Init() initialisiert die OSMP-Umgebung und ermöglicht den Zugang zu den gemeinsamen Ressourcen der OSMP-Prozesse.
 * Sie muss von jedem OSMP-Prozess zu Beginn aufgerufen werden.
 *
 * @param [in] argc Adresse der Argumentzahl
 * @param [in] argv Adresse des Argumentvektors
 *
 * @return OSMP_SUCCESS or OSMP_FAILURE or OSMP_CRITICAL_FAILURE
 */
int OSMP_Init(const int *argc, char ***argv);

/**
 * Die Funktion OSMP_Size() liefert in *size die Zahl der OSMP-Prozesse ohne den OSMP-Starter Prozess zurück.
 * Sollte mit der Zahl übereinstimmen, die in der Kommandozeile dem OSMP-Starter übergeben wird.
 *
 * @param [out] rank Zahl der OSMP-Prozesse
 *
 * @return OSMP_SUCCESS or OSMP_FAILURE or OSMP_CRITICAL_FAILURE
 */
int OSMP_Size(int *size);

/**
 * Die Funktion OSMP_Rank() liefert in *rank die OSMP-Prozessnummer des aufrufenden OSMP-Prozesses von 0,…,np-1 zurück.
 *
 * @param [out] rank Prozessnummer 0,…,np-1 des aktuellen OSMP-Prozesse
 *
 * @return OSMP_SUCCESS or OSMP_FAILURE or OSMP_CRITICAL_FAILURE
 */
int OSMP_Rank(int *rank);

/**
 * Die Funktion OSMP_Send() sendet eine Nachricht an den Prozess mit der Nummer dest. 
 * Die Nachricht besteht aus count Elementen vom Typ datatype. 
 * Die zu sendende Nachricht beginnt im aufrufenden Prozess bei der Adresse buf. 
 * Die Funktion ist blockierend, d.h. wenn sie in das aufrufende Programm zurückkehrt, ist der Kopiervorgang abgeschlossen.
 *
 * @param [in] buf      Startadresse des Puffers mit der zu sendenden Nachricht
 * @param [in] count    Zahl der Elemente vom angegebenen Typ im Puffer
 * @param [in] datatype OSMP-Typ der Daten im Puffer
 * @param [in] dest     Nummer des Empfängers zwischen 0,…,np-1
 *
 * @return OSMP_SUCCESS or OSMP_FAILURE or OSMP_CRITICAL_FAILURE
 */
int OSMP_Send(const void *buf, int count, OSMP_Datatype datatype, int dest);

/**
 * Der aufrufende Prozess empfängt eine Nachricht mit maximal count Elementen des angegebenen Datentyps datatype.
 * Die Nachricht wird an die Adresse buf des aufrufenden Prozesses geschrieben.
 * Unter source wird die OSMP-Prozessnummer des sendenden Prozesses und unter len die tatsächliche Länge der gelesenen Nachricht abgelegt.
 * Die Funktion ist blockierend, d.h. sie wartet, bis eine Nachricht für den Prozess vorhanden ist. 
 * Wenn die Funktion zurückkehrt, ist der Kopierprozess abgeschlossen.
 * Die Nachricht gilt nach dem Aufruf dieser Funktion als abgearbeitet.
 *
 * @param [out] buf      Startadresse des Puffers im lokalen Speicher des aufrufenden Prozesses, in den die Nachricht kopiert werden soll.
 * @param [in]  count    maximale Zahl der Elemente vom angegebenen Typ, die empfangen werden können
 * @param [in]  datatype OSMP-Typ der Daten im Puffer
 * @param [out] source   Nummer des Senders zwischen 0,…,np-1
 * @param [out] len      tatsächliche Länge der empfangenen Nachricht in Byte
 *
 * @return OSMP_SUCCESS or OSMP_FAILURE or OSMP_CRITICAL_FAILURE
 */
int OSMP_Recv(void *buf, int count, OSMP_Datatype datatype, int *source, int *len);

/**
 * Alle OSMP-Prozesse müssen diese Funktion aufrufen, bevor sie sich beenden. 
 * Sie geben damit den Zugriff auf die gemeinsamen Ressourcen frei.
 * Hierbei muss jeder Prozess zuvor alle noch vorhandenen Nachrichten abarbeiten.
 *
 * @return OSMP_SUCCESS or OSMP_FAILURE or OSMP_CRITICAL_FAILURE
 */
int OSMP_Finalize(void);

/**
 * Diese kollektive Funktion blockiert den aufrufenden Prozess.
 * Erst wenn alle anderen Prozesse ebenfalls an der Barriere angekommen sind, laufen die Prozesse weiter.
 *
 * @return OSMP_SUCCESS or OSMP_FAILURE or OSMP_CRITICAL_FAILURE
 */
int OSMP_Barrier(void);

/**
 * Sammelt Daten von allen aufrufenden Prozessen und liefert sie and den Empfängerprozess.
 * Jeder Prozess kann einen anderen Sendebuffer und eine andere Sendeanzahl bereitstellen,
 * der Empfängerbuffer und die Empfängeranzahl müssen jedoch auf allen Prozessen gleich sein.
 *
 * @param [in]  sendbuf   Pointer to the send buffer.
 * @param [in]  sendcount Number of elements in the send buffer.
 * @param [in]  sendtype  MPI datatype of the send buffer elements.
 * @param [out] recvbuf   Pointer to the receive buffer.
 * @param [in]  recvcount Number of elements in the receive buffer.
 * @param [in]  recvtype  MPI datatype of the receive buffer elements.
 * @param [in]  recv      Ist der aufrufende Prozess der Sender
 *
 * @return OSMP_SUCCESS or OSMP_FAILURE or OSMP_CRITICAL_FAILURE
 */
int OSMP_Gather(void *sendbuf, int sendcount, OSMP_Datatype sendtype, void *recvbuf, int recvcount, OSMP_Datatype recvtype, bool recv);

/**
 * Die Funktion sendet eine Nachricht analog zu OSMP_Send(). 
 * Die Funktion kehrt jedoch sofort zurück, ohne dass das Kopieren der Nachricht sichergestellt ist (nicht blockierendes Senden).
 *
 * @param [in] buf Startadresse des Puffers mit der zu sendenden Nachricht
 * @param [in] count Zahl der Elemente vom angegebenen Typ im Puffer
 * @param [in] datatype  OSMP-Typ der Daten im Puffer
 * @param [in] dest PID des Empfängers zwischen 0, …, np-1
 * @param [in, out] request Adresse einer eigenen Datenstruktur, die später verwendet werden kann, um abzufragen, ob die Operation abgeschlossen ist.
 *
 * @return OSMP_SUCCESS or OSMP_FAILURE or OSMP_CRITICAL_FAILURE
 */
int OSMP_Isend(const void *buf, int count, OSMP_Datatype datatype, int dest, OSMP_Request request);

/**
 * Die Funktion empfängt eine Nachricht analog zu OSMP_Recv().
 * Die Funktion kehrt jedoch sofort zurück, ohne dass das Kopieren der Nachricht sichergestellt ist (nicht blockierendes Empfangen).
 *
 * @param [out] buf
 * @param [in] count
 * @param [in] datatype
 * @param [out] source
 * @param [out] len
 * @param [in, out] request Adresse einer Datenstruktur, die später verwendet werden kann, um abzufragen, ob die die Operation abgeschlossen ist.
 *
 * @return OSMP_SUCCESS or OSMP_FAILURE or OSMP_CRITICAL_FAILURE
 */
int OSMP_Irecv(void *buf, int count, OSMP_Datatype datatype, int *source, int *len, OSMP_Request request);

/**
 * Die Funktion testet, ob die mit dem request verknüpfte Operation abgeschlossen ist. 
 * Sie ist nicht blockierend, d.h. sie wartet nicht auf das Ende der mit request verknüpften Operation.
 *
 * @param [in] request Adresse der Struktur, die eine blockierende Operation spezifiziert
 * @param [out] flag Gibt den Status der Operation an.
 *
 * @return OSMP_SUCCESS or OSMP_FAILURE or OSMP_CRITICAL_FAILURE
 */
int OSMP_Test(OSMP_Request request, int *flag);

/**
 * Die Funktion prüft, ob die mit dem request verknüpfte, nicht blockierende Operation abgeschlossen ist.
 * Sie ist so lange blockiert, bis dies der Fall ist.
 *
 * @param [in] request Adresse der Struktur, die eine nicht blockierende Operation spezifiziert
 *
 * @return OSMP_SUCCESS or OSMP_FAILURE or OSMP_CRITICAL_FAILURE
 */
int OSMP_Wait (OSMP_Request request);

/**
 * Die Funktionen stellen den Speicher für einen Request zur Verfügung bzw. deallozieren den Speicher.
 *
 * @param [out] request Adresse eines Requests (input)
 *
 * @return OSMP_SUCCESS or OSMP_FAILURE or OSMP_CRITICAL_FAILURE
 */
int OSMP_CreateRequest(OSMP_Request *request);

/**
 * Die Funktionen stellen den Speicher für einen Request zur Verfügung bzw. deallozieren den Speicher.
 *
 * @param [in] request Adresse eines Requests
 *
 * @return OSMP_SUCCESS or OSMP_FAILURE or OSMP_CRITICAL_FAILURE
 */
int OSMP_RemoveRequest(OSMP_Request *request);

/**
 * Diese Funktion gibt den Namen des Shared Memory Bereichs im Parameter name zurück.
 *
 * @param [out] name Der Name des Shared Memory Bereichs
 *
 * @return OSMP_SUCCESS or OSMP_FAILURE or OSMP_CRITICAL_FAILURE
 */
int OSMP_GetShmName(char** name);

#endif /* BETRIEBSSYSTEME_OSMP_H */
