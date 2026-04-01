# Lab3Redes# Laboratorio 3 — Publish/Subscribe con TCP y UDP

## Descripción

En este laboratorio se implementa un sistema de comunicación basado en el modelo publish/subscribe usando sockets en C sobre Linux. Se desarrollan dos versiones del sistema:

* TCP: protocolo confiable y orientado a conexión
* UDP: protocolo no confiable y no orientado a conexión

El sistema tiene tres componentes:

* Publisher: genera eventos asociados a un tema (por ejemplo, un partido)
* Broker: recibe los mensajes y los distribuye a los suscriptores correspondientes
* Subscriber: se suscribe a uno o más temas y recibe los mensajes

---

## Estructura del proyecto

```
Lab3Redes/
│
├── TCP/
│   ├── broker_tcp.c
│   ├── publisher_tcp.c
│   ├── subscriber_tcp.c
│
├── UDP/
│   ├── broker_udp.c
│   ├── publisher_udp.c
│   ├── subscriber_udp.c
│   ├── flood_publisher_udp.c
│
└── wireshark/
    ├── tcp_pubsub.pcap
    └── udp_pubsub.pcap
```

---

## Requisitos

* Linux
* GCC
* tcpdump o Wireshark

Instalación:

```
sudo apt update
sudo apt install build-essential tcpdump wireshark
```

---

## TCP

### Compilación

```
cd TCP
gcc broker_tcp.c -o broker_tcp
gcc publisher_tcp.c -o publisher_tcp
gcc subscriber_tcp.c -o subscriber_tcp
```

### Ejecución

En terminales separadas:

```
./broker_tcp
```

```
./subscriber_tcp PartidoA
```

```
./subscriber_tcp PartidoA
```

```
./publisher_tcp PartidoA
```

### Captura de tráfico

```
sudo tcpdump -i lo tcp port 9000 -w tcp_pubsub.pcap
```

---

## UDP

### Compilación

```
cd UDP
gcc broker_udp.c -o broker_udp
gcc publisher_udp.c -o publisher_udp
gcc subscriber_udp.c -o subscriber_udp
gcc flood_publisher_udp.c -o flood_publisher_udp
```

### Ejecución

```
./broker_udp
```

```
./subscriber_udp PartidoA
```

```
./subscriber_udp PartidoA PartidoB
```

```
./publisher_udp PartidoA
```

### Prueba de saturación

```
./flood_publisher_udp PartidoA 50000
```

### Captura de tráfico

```
sudo tcpdump -i lo udp port 5000 -w udp_pubsub.pcap
```

---

## Análisis

### Confiabilidad

TCP utiliza confirmaciones (ACK) y retransmisión de paquetes, por lo que garantiza la entrega.
UDP no tiene mecanismos de confirmación, por lo que no garantiza que los mensajes lleguen.

---

### Orden de entrega

TCP asegura el orden mediante números de secuencia.
UDP no tiene control de orden, los mensajes pueden llegar desordenados.

---

### Pérdida de mensajes

En TCP, los paquetes perdidos se retransmiten automáticamente.
En UDP, si un paquete se pierde, no se recupera.

Durante las pruebas se observaron mensajes ICMP indicando “destination unreachable”, lo cual evidencia que UDP no valida la existencia del receptor.

---

### Overhead

TCP tiene mayor overhead debido a sus cabeceras y mecanismos de control.
UDP tiene menor overhead, lo que lo hace más eficiente pero menos confiable.

---

## Conclusión

TCP es adecuado cuando se necesita confiabilidad y orden en la comunicación.
UDP es más eficiente y rápido, pero no garantiza entrega ni orden, por lo que es más apropiado en escenarios donde la pérdida de algunos datos es tolerable.

---

## Autor

Grupo 4

---
