### Slave Rang
| Value   | Meaning                                      |
| ------- | -------------------------------------------- |
| 1-247   | Valid slave addresses                        |
| 0       | Broadcast (all devices listen, none respond) |
| 248-255 | Reserved                                     |

Typeical setup:
- Water meter: Slave ID 1
- Electric meter: Slave ID 2
- Other meter: ...
