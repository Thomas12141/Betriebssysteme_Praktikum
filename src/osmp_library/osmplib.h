#ifndef BETRIEBSSYSTEME_OSMPLIB_H
#define BETRIEBSSYSTEME_OSMPLIB_H
#include <stdio.h>

#include "OSMP.h"

/**
 * Dieses Makro wird verwendet, um den Compiler davon zu überzeugen, dass eine Variable verwendet wird.
 */
#define UNUSED(x) { (void)(x); }

/**
 * Mit diesem Wert wird signalisiert, dass ein Prozess keine weitere Nachricht hat.
 */
#define NO_MESSAGE 0

/**
 * Mit diesem Wert werden alle nicht genutzten Elemente des Arrays belegt, in dem die freien Nachrichtenslots verzeichnet sind.
 */
#define NO_SLOT 0

/**
 * Struct für eine Nachricht entsprechend der Definition unseres Shared Memory.
 */
typedef struct OSMP_message {
    unsigned short free;                    // Flag: Slot frei (1) oder nicht (0)?
    OSMP_Datatype type;                     // Datentyp der enthaltenen Nachricht
    char payload[OSMP_MAX_PAYLOAD_LENGTH];  // eigentliche Nachricht
    int next_message;                       // Offset (rel. zum Anfang des SHM) zur nächsten Nachricht des Empfängers (NO_MESSAGE = keine weitere Nachricht)
} OSMP_message;

#endif //BETRIEBSSYSTEME_OSMPLIB_H
