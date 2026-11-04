# Architecture

## Entities (back-end relationship model)

- An **Admin** owns **Device Types** (for example, "RFID device" or "Fingerprint device").
- A **Device Type** owns **Devices** and has **Commands** (`getenrolluser`, `enroll`, `match`, `userdelete`, ...).
- A **Command** allows **Parameters** and maps to a **Form** in the business app (for example, `match` → HR "Add attendance").

## Command flow

1. The back end queues a command for a device (identified by a `devicerelationid` GUID).
2. The device polls `.../rfid.aspx?devicerid=<id>&getpendingcommands=true`, or the server pushes the command to the device's
   `/executeCommand` endpoint.
3. The device runs the command (for example, enrolling a finger and returning a template ID).
4. The device reports the result: `.../rfid.aspx?devicerelationid=<id>&historyid=<n>&recorded=<enroll|fail|...>&output=<value>`.
5. For `match`, the device sends `Parameterdata=Date;<d>$Time;<t>$Location;<loc>$Useraddress;<user>`. The server writes this
   to the mapped form (an attendance record).

Listeners (`/addListner`) let one device forward match events to several service URLs.
