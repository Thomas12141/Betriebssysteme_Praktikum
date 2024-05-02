/******************************************************************************
* FILE: OSMP.h
* DESCRIPTION:
* Beinhaltet alle in der Anleitung angegeben Prototypen der OSMP Kernfunktionen wie z.B. OSMP_Test() und vorgegebene Konstanten.
*
* LAST MODIFICATION: March 07, 2024
******************************************************************************/
#ifndef BETRIEBSSYSTEME_OSMP_H
#define BETRIEBSSYSTEME_OSMP_H

/**
 * Alle OSMP-Funktionen liefern im Erfolgsfall OSMP_SUCCESS als Rückgabewert. 
 * Weitere Rückgabewerte können mit Begründung (und Dokumentation!) definiert werden
 */
#define OSMP_SUCCESS 0

/**
 * Im Fehlerfall liefern die OSMP-Funktionen den Wert OSMP_FAILURE zurück. 
 * Die Fehler führen aber nicht zum beenden des Programms (z. B. wenn ein Prozess eine Nachricht an einen nicht existierenden Prozess schickt).
 */
#define OSMP_FAILURE ( !OSMP_SUCCESS )

typedef void* OSMP_Request;

/**
 * Die maximale Zahl der Nachrichten pro Prozess 
 */
#define OSMP_MAX_MESSAGES_PROC 16

/**
 * Die maximale Anzahl der Nachrichten, die insgesamt vorhanden sein dürfen
 */
#define OSMP_MAX_SLOTS 256

/**
 * Die maximale Länge der Nutzlast einer Nachricht
 */
#define OSMP_MAX_PAYLOAD_LENGTH 1024

/**
 * Die OSMP-Datentypen entsprechen den C-Datentypen. 
 * Sie werden verwendet, um den Typ der Daten anzugeben, die mit den OSMP-Funktionen gesendet bzw. empfangen werden sollen.
 */
typedef enum OSMP_Datatype {
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
int get_OSMP_MAX_PAYLOAD_LENGTH();

/**
 * Gibt die Maximale Anzahl der Nachrichten, die insgesamt vorhanden sein dürfen zurück. 
 */
int get_OSMP_MAX_SLOTS();

/**
 * Gibt die maximale Zahl der Nachrichten pro Prozess zurück.
 */
int get_OSMP_MAX_MESSAGES_PROC();

/**
 * Gibt den Wert von OSMP_FAILURE zurück.
 */
int get_OSMP_FAILURE();

/**
 * Gibt den Wert von OSMP_SUCCESS zurück.
 */
int get_OSMP_SUCCESS();

/**
 * Die Funktion OSMP_SizeOf() liefert in *size* die Größe des Datentyps datatype in Byte zurück.
 *
 * @param [in] datatype OSMP-Datentyp
 * @param [out] size Größe des Datentyps in Byte
 *
 * @return Im Erfolgsfall OSMP_SUCCESS; falls der OSMP_Datatype nicht existiert, OSMP_FAILURE
 */
int OSMP_SizeOf(OSMP_Datatype datatype, unsigned int *size);

/**
 * Die Funktion OSMP_Init() initialisiert die OSMP-Umgebung und ermöglicht den Zugang zu den gemeinsamen Ressourcen der OSMP-Prozesse.
 * Sie muss von jedem OSMP-Prozess zu Beginn aufgerufen werden.
 * Durch diesen Aufruf wird außerdem der Posteingang des Prozesses freigegeben.
 *
 * @param [in] argc Adresse der Argumentzahl
 * @param [in] argv Adresse des Argumentvektors
 *
 * @return Im Erfolgsfall OSMP_SUCCESS, sonst OSMP_FAILURE
 */
int OSMP_Init(const int *argc, char ***argv);

/**
 * Die Funktion OSMP_Size() liefert in *size* die Zahl der OSMP-Prozesse ohne den OSMP-Starter Prozess zurück.
 * Sollte mit der Zahl übereinstimmen, die in der Kommandozeile dem OSMP-Starter übergeben wird.
 *
 * @param [out] size Zahl der OSMP-Prozesse
 *
 * @return Im Erfolgsfall OSMP_SUCCESS, sonst OSMP_FAILURE
 */
int OSMP_Size(int *size);

/**
 * Die Funktion OSMP_Rank() liefert in *rank die OSMP-Prozessnummer des aufrufenden OSMP-Prozesses von 0,…,np-1 zurück.
 *
 * @param [out] rank Prozessnummer 0,…,np-1 des aktuellen OSMP-Prozesse
 *
 * @return Im Erfolgsfall OSMP_SUCCESS, sonst OSMP_FAILURE
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
 * @return Im Erfolgsfall OSMP_SUCCESS, sonst OSMP_FAILURE
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
 * @return Im Erfolgsfall OSMP_SUCCESS, sonst OSMP_FAILURE
 */
int OSMP_Recv(void *buf, int count, OSMP_Datatype datatype, int *source, int *len);

/**
 * Alle OSMP-Prozesse müssen diese Funktion aufrufen, bevor sie sich beenden. 
 * Sie geben damit den Zugriff auf die gemeinsamen Ressourcen frei.
 * Hierbei muss jeder Prozess zuvor alle noch vorhandenen Nachrichten abarbeiten. Dies bedeutet, dass der Posteingang gesperrt wird und alle noch vorhandenen Nachrichten gelöscht werden.
 *
 * @return Im Erfolgsfall OSMP_SUCCESS, sonst OSMP_FAILURE
 */
int OSMP_Finalize(void);

/**
 * Diese kollektive Funktion blockiert den aufrufenden Prozess.
 * Erst wenn alle anderen Prozesse ebenfalls an der Barriere angekommen sind, laufen die Prozesse weiter.
 *
 * @return Im Erfolgsfall OSMP_SUCCESS, sonst OSMP_FAILURE
 */
int OSMP_Barrier(void);

/** 
 * Diese Funktion ermöglicht die Gather-Kommunikation. 
 * Hierbei können mehrere Prozesse an einen Empfänger Prozess Daten schicken.
 *
 * @param [in]  sendbuf   Zeiger auf den Sendepuffer.
 * @param [in]  sendcount Anzahl der Elemente im Sendepuffer.
 * @param [in]  sendtype  OSMP-Datentyp der Elemente im Sendepuffer.
 * @param [out] recvbuf   Zeiger auf den Empfangspuffer.
 * @param [in]  recvcount Anzahl der Elemente im Empfangspuffer.
 * @param [in]  recvtype  OSMP-Datentyp der Elemente im Empfangspuffer.
 * @param [in]  recv      1, falls der aufrufende Prozess der Empfänger ist, sonst 0.
 *
 * @return Im Erfolgsfall OSMP_SUCCESS, sonst OSMP_FAILURE
 */
int OSMP_Gather(void *sendbuf, int sendcount, OSMP_Datatype sendtype, void *recvbuf, int recvcount, OSMP_Datatype recvtype, int recv);

/**
 * Die Funktion sendet eine Nachricht analog zu OSMP_Send(). 
 * Die Funktion kehrt jedoch sofort zurück, ohne dass das Kopieren der Nachricht sichergestellt ist (nicht blockierendes Senden).
 *
 * @param [in]      buf Startadresse des Puffers mit der zu sendenden Nachricht
 * @param [in]      count Zahl der Elemente vom angegebenen Typ im Puffer
 * @param [in]      datatype  OSMP-Typ der Daten im Puffer
 * @param [in]      dest PID des Empfängers zwischen 0, …, np-1
 * @param [in, out] request Adresse einer eigenen Datenstruktur, die später verwendet werden kann, um abzufragen, ob die Operation abgeschlossen ist.
 *
 * @return Im Erfolgsfall OSMP_SUCCESS, sonst OSMP_FAILURE
 */
int OSMP_ISend(const void *buf, int count, OSMP_Datatype datatype, int dest, OSMP_Request request);

/**
 * Die Funktion empfängt eine Nachricht analog zu OSMP_Recv().
 * Die Funktion kehrt jedoch sofort zurück, ohne dass das Kopieren der Nachricht sichergestellt ist (nicht blockierendes Empfangen).
 *
 * @param [out]     buf Startadresse des Speicherbereichs, wo die zu empfangende Nachricht gespeichert werden soll.
 * @param [in]      count Zahl der Elemente vom angegebenen Typ, die empfangen werden können
 * @param [in]      datatype OSMP-Typ der Daten im Puffer
 * @param [out]     source PID des Senders zwischen 0, …, np-1
 * @param [out]     len tatsächliche Länge der empfangenen Nachricht in Byte
 * @param [in, out] request Adresse einer Datenstruktur, die später verwendet werden kann, um abzufragen, ob die die Operation abgeschlossen ist.
 *
 * @return Im Erfolgsfall OSMP_SUCCESS, sonst OSMP_FAILURE
 */
int OSMP_IRecv(void *buf, int count, OSMP_Datatype datatype, int *source, int *len, OSMP_Request request);

/**
 * Die Funktion testet, ob die mit der Request verknüpften Operation abgeschlossen ist. 
 * Sie ist nicht blockierend, d.h. sie wartet nicht auf das Ende der mit request verknüpften Operation.
 *
 * @param [in]  request Adresse der Struktur, die eine blockierende Operation spezifiziert
 * @param [out] flag Gibt den Status der Operation an.
 *
 * @return Im Erfolgsfall OSMP_SUCCESS, sonst OSMP_FAILURE
 */
int OSMP_Test(OSMP_Request request, int *flag);

/**
 * Die Funktion wartet, bis die mit der Request verknüpfte, nicht blockierende Operation abgeschlossen ist.
 * Sie ist so lange blockiert, bis dies der Fall ist.
 *
 * @param [in] request Adresse der Struktur, die eine nicht blockierende Operation spezifiziert
 *
 * @return Im Erfolgsfall OSMP_SUCCESS, sonst OSMP_FAILURE
 */
int OSMP_Wait(OSMP_Request request);

/**
 * Erstellt eine OSMP_Request.
 * Eine OSMP_Request wird dazu verwendet, um nicht blockierende Operationen zu überwachen.
 *
 * @param [out] request Adresse eines Requests (input)
 *
 * @return Im Erfolgsfall OSMP_SUCCESS, sonst OSMP_FAILURE
 */
int OSMP_CreateRequest(OSMP_Request *request);

/**
 * Löscht eine OSMP_Request.
 *
 * @param [in] request Adresse eines Requests
 *
 * @return Im Erfolgsfall OSMP_SUCCESS, sonst OSMP_FAILURE
 */
int OSMP_RemoveRequest(OSMP_Request *request);

/**
 * Gibt den Namen des Shared Memory Bereichs zurück.
 *
 * @param [out] name Der Name des Shared Memory Bereichs
 *
 * @return Im Erfolgsfall OSMP_SUCCESS, sonst OSMP_FAILURE
 */
int OSMP_GetSharedMemoryName(char **name);

void OSMP_GetSharedMemoryPointer(char **shared_memory);
#endif /* BETRIEBSSYSTEME_OSMP_H */
