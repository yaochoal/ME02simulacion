
# ME02 : Simulador NS3.

## Propósito

Generar una Red Ad hoc de mínimo 20 nodos, con diferentes tipos de tráfico y servicios, se debe utilizar la herramienta OpenAI Gym 
definiendo dos métricas de aprendizaje para la simulación creada.

## Integrantes

|       Integrante      |                 Correo                       |
|-----------------------|-----------------------------------------------|
| Dave Sebastian Valencia Salazar      |    <dsvalencias@unal.edu.co>    |
| Yesid Alberto Ochoa Luque      |    <yaochoal@unal.edu.co>     |

## Entregables

### 1. Código fuente del simulador construido:


``` fichero: ./red-ad-hoc.cc```
```
#include "ns3/core-module.h"
#include "ns3/applications-module.h"
#include "ns3/mobility-module.h"
#include "ns3/wifi-module.h"
#include "ns3/csma-module.h"
#include "ns3/olsr-helper.h"
#include "ns3/internet-module.h"
#include "ns3/netanim-module.h"

using namespace ns3;

//
// Definir palabra clave de registro para este archivo
//
NS_LOG_COMPONENT_DEFINE ("RedAdHoc");


static void
CourseChangeCallback (std::string path, Ptr<const MobilityModel> model)
{
  Vector position = model->GetPosition ();
  std::cout << "CourseChange " << path << " x=" << position.x << ", y=" << position.y << ", z=" << position.z << std::endl;
}

int
main (int argc, char *argv[])
{
  //
  // Primero, inicializamos algunas variables locales que controlan algunos
  // parametros de la simulacion.
  //
  uint32_t backboneNodes = 25;
  uint32_t infraNodes = 1;
  uint32_t lanNodes = 1;
  uint32_t stopTime = 20;
  bool useCourseChangeCallback = false;

  Config::SetDefault ("ns3::OnOffApplication::PacketSize", StringValue ("1472"));
  Config::SetDefault ("ns3::OnOffApplication::DataRate", StringValue ("100kb/s"));

  CommandLine cmd;
  cmd.AddValue ("backboneNodes", "number of backbone nodes", backboneNodes);
  cmd.AddValue ("infraNodes", "number of leaf nodes", infraNodes);
  cmd.AddValue ("lanNodes", "number of LAN nodes", lanNodes);
  cmd.AddValue ("stopTime", "simulation stop time (seconds)", stopTime);
  cmd.AddValue ("useCourseChangeCallback", "whether to enable course change tracing", useCourseChangeCallback);

  cmd.Parse (argc, argv);

  if (stopTime < 10)
    {
      std::cout << "Use a simulation stop time >= 10 seconds" << std::endl;
      exit (1);
    }
  ///////////////////////////////////////////////////////////////////////////
  //                                                                       //
  // EL backbone                                                           //
  //                                                                       //
  ///////////////////////////////////////////////////////////////////////////

  //
  // Creamos un contenedor para administrar los nodos de la red adhoc (backbone).
  //
  NodeContainer backbone;
  backbone.Create (backboneNodes);
  //
  // Creamos los dispositivos de red wifi y los instalamos
  // en nuestro contenedor
  //
  WifiHelper wifi;
  WifiMacHelper mac;
  mac.SetType ("ns3::AdhocWifiMac");
  wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager",
                                "DataMode", StringValue ("OfdmRate54Mbps"));
  YansWifiPhyHelper wifiPhy = YansWifiPhyHelper::Default ();
  YansWifiChannelHelper wifiChannel = YansWifiChannelHelper::Default ();
  wifiPhy.SetChannel (wifiChannel.Create ());
  NetDeviceContainer backboneDevices = wifi.Install (wifiPhy, mac, backbone);

  NS_LOG_INFO ("Enabling OLSR routing on all backbone nodes");
  OlsrHelper olsr;
  //
  // Agregamos la pila de protocolos IPv4 a los nodos en nuestro contenedor
  //
  InternetStackHelper internet;
  internet.SetRoutingHelper (olsr); 
  internet.Install (backbone);

  //
  // Asignamos direcciones IPv4 a los controladores de dispositivo (en realidad a las interfaces 
  // IPv4 asociadas) que acabamos de crear.
  //
  Ipv4AddressHelper ipAddrs;
  ipAddrs.SetBase ("192.168.0.0", "255.255.255.0");
  ipAddrs.Assign (backboneDevices);

  //
  // Los nodos de red ad-hoc necesitan un modelo de movilidad, por lo que agregamos uno para
  // cada uno de los nodos que acabamos de terminar de construir.
  //
  MobilityHelper mobility;
  mobility.SetPositionAllocator ("ns3::GridPositionAllocator",
                                 "MinX", DoubleValue (20.0),
                                 "MinY", DoubleValue (20.0),
                                 "DeltaX", DoubleValue (20.0),
                                 "DeltaY", DoubleValue (20.0),
                                 "GridWidth", UintegerValue (5),
                                 "LayoutType", StringValue ("RowFirst"));
  mobility.SetMobilityModel ("ns3::RandomDirection2dMobilityModel",
                             "Bounds", RectangleValue (Rectangle (-500, 500, -500, 500)),
                             "Speed", StringValue ("ns3::ConstantRandomVariable[Constant=2]"),
                             "Pause", StringValue ("ns3::ConstantRandomVariable[Constant=0.2]"));
  mobility.Install (backbone);

  if (useCourseChangeCallback == true)
    {
      Config::Connect ("/NodeList/*/$ns3::MobilityModel/CourseChange", MakeCallback (&CourseChangeCallback));
    }

  AnimationInterface anim ("red-ad-hoc.xml");

  ///////////////////////////////////////////////////////////////////////////
  //                                                                       //
  // Corremos la simulacion                                                      //
  //                                                                       //
  ///////////////////////////////////////////////////////////////////////////

  NS_LOG_INFO ("Run Simulation.");
  Simulator::Stop (Seconds (stopTime));
  Simulator::Run ();
  Simulator::Destroy ();
}
```
### 1.1 Descripción del codigo: 

Para comprender la simulación como un todo, primero se debe explicar cómo funciona la red ad-hoc a tratar, esta se compone de una serie de rutinas que descomponen la funcionalidad en dos submodelos, uno de comunicación y el otro de movilidad de los nodos o terminales que intercambian información, los cuales siguen una parametrización en cantidad de nodos a trabajar, tamaño, tipo y tasa de transmisión de paquetes. 

Para definir el submodelo de comunicación se genera un espacio que delimita los nodos, luego crea la cantidad de nodos parametrizados, a estos nodos se le impone el uso de red wifi y se otorgan características a cada uno de los nodos tales como dirección MAC, tipo de red y canal a usar. Más tarde, se implementa el protocolo de internet (IPv4) en cada dispositivo, y finalmente se le asigna una dirección IPv4. Como toda red ad-hoc requiere un componente de movilidad, se integra un submodelo que lo contemple, en este caso, se aplica un modelo de movilidad en 2D delimitando una región frontera; y asignando variables de velocidad, posición (en el eje X y Y) en cada uno de los nodos con el fin de observar recorridos. 

Finalmente, al tener los dos submodelos que satisfacen el estudio de una red ad-hoc, se corre la simulación y se despliega en un componente de visualización.

``` fichero: ./simple_test.py```
```
#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import gym
import argparse
import ns3gym

env = gym.make('ns3-v0')
env.reset()

ob_space = env.observation_space
ac_space = env.action_space
print("Observation space: ", ob_space,  ob_space.dtype)
print("Action space: ", ac_space, ac_space.dtype)

stepIdx = 0

try:
    obs = env.reset()
    print("Step: ", stepIdx)
    print("---obs: ", obs)

    while True:
        stepIdx += 1

        action = env.action_space.sample()
        print("---action: ", action)
        obs, reward, done, info = env.step(action)

        print("Step: ", stepIdx)
        print("---obs, reward, done, info: ", obs, reward, done, info)

        if done:
            break

except KeyboardInterrupt:
    print("Ctrl-C -> Exit")
finally:
    env.close()
    print("Done")
```

### 1.2 Descripción del codigo: 




### 2. Manual técnico:

### Requerimientos: 

- Ubuntu 16.04
- NS3
- OpenAI Gym

### Herramientas utilizadas en el desarrollo: 

#### NS3: 

Este es un <<framework>> el cual está dedicado exclusivamente a la aplicación de técnicas de inteligencia artificial, cuyo objetivo es estandarizar las rutinas aplicadas por la comunidad con esto se pueden replicar experimentos y acelerar los avances en el campo de la inteligencia artificial.

#### OpenAI Gym:

Este software se encarga principalmente de simular redes de computadores, cuya extensión aplica principalmente al estudio de redes móviles ad-hoc al permitir la implementación de protocolos de redes cableadas y de redes inalámbricas.

## 3. Resultados.
