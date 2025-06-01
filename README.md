# SES – Smart Event System
## Übersicht
SES (Smart Event System) ist ein leichtgewichtiges, prioritätsbasiertes Event-System für asynchrone und parallele Verarbeitung von Nachrichten (Events). 
Es ermöglicht das Einreihen, Verarbeiten und Verwerfen von Nachrichten, sortiert nach Priorität. SES ist so konzipiert, dass mehrere parallele Tasks unterschiedliche Prioritätsbereiche 
der Nachrichten verarbeiten können.

## Kernkonzept

- Nachrichten (Messages): Werden mit einer Priorität versehen. Kleinere Prioritätswerte bedeuten höhere Priorität.
- Eventmanager: Verwaltet die Nachrichtenliste, die intern stets nach Priorität sortiert ist.
- Prioritätsbereich: Tasks können spezifische Prioritätsbereiche anfragen und nur diese Nachrichten abarbeiten.
- Mehrere Tasks: Mehrere Tasks können gleichzeitig arbeiten, jeder in seinem Prioritätsbereich.
- Thread-Sicherheit: Synchronisation mit einem timed_countlock, um parallelen Zugriff zu steuern.
- Nachrichten-Lebenszyklus: Nachrichten können als fertig (marked), verarbeitet, abgelehnt (discarded) oder gelöscht werden.
— kleinere Werte bedeuten dabei höhere Priorität. Tasks können beliebige Prioritätsbereiche abdecken, um parallel verschiedene Eventgruppen zu verarbeiten.

### Event Groups

Events können in **EventGroups** organisiert sein. Eine Event Group ist eine logische Zusammenfassung mehrerer Events, z. B. um zusammengehörige Ereignisse aus einem externen System oder 
verschiedenen Quellen zu bündeln. Das System behandelt Event Groups konsistent und ermöglicht so eine klare Strukturierung und Verwaltung komplexer Eventlandschaften.


## Funktionen des Eventmanagers
Die Verarbeitung erfolgt in klar definierten Phasen, um parallele und sichere Zugriffe zu ermöglichen:

- **beginMessages**: Ein Task signalisiert den Start der Verarbeitung. Dabei wird ein Lock gesetzt, das den Zugriff auf die Eventliste schützt.
- **processMessages**: Der Task verarbeitet alle Events, die in seinem Prioritätsbereich liegen (z. B. von Priorität 1 bis 7).
- **endProcessMessages**: Nach der Verarbeitung werden alle Events, die als erledigt (markiert) gelten, entfernt, und der Lock wird freigegeben.

Durch dieses Modell können mehrere Tasks gleichzeitig Events aus unterschiedlichen Prioritätsbereichen parallel bearbeiten, ohne sich gegenseitig zu blockieren.


```
void postMessage(message_ptr msg, uint64_t maxWaitTime);
```
Fügt eine neue Nachricht in das System ein. Die Nachricht wird automatisch nach ihrer Priorität einsortiert. 
Wenn der Lock nicht innerhalb von maxWaitTime erlangt wird, wird die Nachricht mit onMessagePost über den Fehlschlag informiert.

```
bool beginMessages();
bool processMessages(int fromPrio, int toPrio);
bool processMessages(uint8_t prio);
bool endProcessMessages();
```
- beginMessages(): Signalisiert den Start der Nachrichtenverarbeitung (erhöht den Lock-Zähler).
- processMessages(fromPrio, toPrio): Verarbeitet alle Nachrichten mit Prioritäten im Bereich [fromPrio, toPrio].
- Nachrichten, die abgelaufen oder abgeschlossen sind, werden entsprechend behandelt.
- processMessages(prio): Kürzere Version, um Nachrichten mit einer einzelnen Priorität zu verarbeiten.
- endProcessMessages(): Entfernt alle beendete Nachrichten aus der Liste und gibt den Lock frei.

## Nachrichten verwerfen (Discard)

```
void discardMessage(const message_ptr& msg);
```

Markiert eine Nachricht als verworfen. Wenn eine Nachricht zu oft verworfen wurde (5 Mal oder mehr), 
wird sie aus dem Eventmanager gelöscht und onMessageDiscard wird aufgerufen.


## Nachrichtenstatus
- Nachrichten können mit set_marked(), is_marked() als fertig makiert werden, werden in endProgressMessage gelöscht
- Nachrichten können ablaufen (is_expired()).
- Nachrichten melden selbst, ob sie verarbeitet wurden (onMessageProcess()).
- Nachrichten behalten eine Zähler-Logik für Verwerfungen.

## Typische Nutzung
```
// Event-Manager erzeugen mit maximaler Wartezeit für Lock (z.B. 100ms)
ses::eventmanager manager(100);

// Nachrichten posten
manager.postMessage(msg, 50);

// Task A verarbeitet Prioritäten 1-7
manager.beginMessages();
manager.processMessages(1, 7);
manager.endProcessMessages();

// Task B verarbeitet Prioritäten 3-28
manager.beginMessages();
manager.processMessages(3, 28);
manager.endProcessMessages();
```

## Vorteile

- Prioritätsbasiert: Nachrichten werden strikt nach Priorität verarbeitet.
- Parallele Verarbeitung möglich: Tasks können unterschiedliche Prioritätsbereiche abarbeiten.
- Thread-sicher: Zugriff auf Nachrichtenliste ist synchronisiert.
- Flexibel und einfach: Keine komplexen IDs, Dispatcher oder Listener. Nur Posten und Verarbeiten.

License
This project is licensed under the EUPL-1.2. Please see [LICENSE](LICENSE) for more details.

