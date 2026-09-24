# RS485
Define based on meter-reader via reading daemon that polls meters via Modbus TCP

- automate: 10m
- cost savings via deadband filtering (60-80% fewer database writes)
- real-time monitoring


### Requirement:

| Component  | Technology                | Rationale                                         |
| ---------- | ------------------------- | ------------------------------------------------- |
| Language   | C++17 + C11               | Production-grade, zero CVEs, minimal dependencies |
| Protocol   | Modbus TCP                | Industry standard for industrial meters           |
| Gateway    | E810-DTU (RS485→Ethernet) | Automatic Modbus TCP→RTU conversion               |
| Database   | MySQL 8.0                 | Existing infrastructure, sufficient for scale     |
| Build      | CMake 3.16+               | Modern, modular, cross-platform                   |
| Deployment | Docker + Docker Compose   | Reproducible, easy deployment                     |