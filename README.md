
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

### 1. Código fuente del modelo 1 red ad-hoc sin OpenAI Gym:


``` fichero: ./modelo_AddHoc_OnlyNs3/red-ad-hoc.cc```
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

Inicialmente se creo el primer modelo de red Ad Hoc sin incluir OpenAI Gym para comprender la simulación como un todo, primero se debe explicar cómo funciona la red ad-hoc a tratar, esta se compone de una serie de rutinas que descomponen la funcionalidad en dos submodelos, uno de comunicación y el otro de movilidad de los nodos o terminales que intercambian información, los cuales siguen una parametrización en cantidad de nodos a trabajar, tamaño, tipo y tasa de transmisión de paquetes. 

Para definir el submodelo de comunicación se genera un espacio que delimita los nodos, luego crea la cantidad de nodos parametrizados, a estos nodos se le impone el uso de red wifi y se otorgan características a cada uno de los nodos tales como dirección MAC, tipo de red y canal a usar. Más tarde, se implementa el protocolo de internet (IPv4) en cada dispositivo, y finalmente se le asigna una dirección IPv4. Como toda red ad-hoc requiere un componente de movilidad, se integra un submodelo que lo contemple, en este caso, se aplica un modelo de movilidad en 2D delimitando una región frontera; y asignando variables de velocidad y posición (en el eje X y Y) en cada uno de los nodos con el fin de observar recorridos. 


### 1.2 Código fuente del modelo 1 red ad-hoc con OpenAI Gym:

``` fichero: ./modelo_addHoc&OpenGym_1/sim.cc```

```
#include "ns3/core-module.h"
#include "ns3/opengym-module.h"
#include "ns3/applications-module.h"
#include "ns3/mobility-module.h"
#include "ns3/wifi-module.h"
#include "ns3/csma-module.h"
#include "ns3/olsr-helper.h"
#include "ns3/internet-module.h"
#include "ns3/netanim-module.h"
using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("OpenGym");

/*
Definimos espacio de observacion
*/
Ptr<OpenGymSpace> MyGetObservationSpace(void)
{
  uint32_t nodeNum = 5;
  float low = 0.0;
  float high = 10.0;
  std::vector<uint32_t> shape = {nodeNum,};
  std::string dtype = TypeNameGet<uint32_t> ();
  Ptr<OpenGymBoxSpace> space = CreateObject<OpenGymBoxSpace> (low, high, shape, dtype);
  NS_LOG_UNCOND ("MyGetObservationSpace: " << space);
  return space;
}

/*
Definimos espacio de acción
*/
Ptr<OpenGymSpace> MyGetActionSpace(void)
{
  uint32_t nodeNum = 2;

  Ptr<OpenGymDiscreteSpace> space = CreateObject<OpenGymDiscreteSpace> (nodeNum);
  NS_LOG_UNCOND ("MyGetActionSpace: " << space);
  return space;
}

/*
Definimos terminacion de juego
*/
bool MyGetGameOver(void)
{

  bool isGameOver = false;
  bool test = false;
  static float stepCounter = 0.0;
  stepCounter += 1;
  if (stepCounter == 10 && test) {
      isGameOver = true;
  }
  NS_LOG_UNCOND ("MyGetGameOver: " << isGameOver);
  return isGameOver;
}

/*
Recojemos Observaciones
*/
Ptr<OpenGymDataContainer> MyGetObservation(void)
{
  //
  // Primero, inicializamos algunas variables locales que controlan algunos
  // parametros de la simulacion.
  //
  uint32_t backboneNodes = 2;
  
  Config::SetDefault ("ns3::OnOffApplication::PacketSize", StringValue ("1472"));
  Config::SetDefault ("ns3::OnOffApplication::DataRate", StringValue ("100kb/s"));
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
  //
  // Agregamos la pila de protocolos IPv4 a los nodos en nuestro contenedor
  //

  //OlsrHelper olsr;
  InternetStackHelper internet;

  //internet.SetRoutingHelper (olsr); 

  internet.Install(backbone);
  //
  // Asignamos direcciones IPv4 a los controladores de dispositivo (en realidad a las interfaces 
  // IPv4 asociadas) que acabamos de crear.
  //
  Ipv4AddressHelper ipAddrs;
  ipAddrs.SetBase ("192.168.0.0", "255.255.255.0");
  //ipAddrs.Assign(backboneDevices);

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

  NS_LOG_INFO ("Run Simulation.");

  //Simulator::Stop (Seconds (stopTime));
  //Simulator::Run ();
  //Simulator::Destroy ();


  //////////////////////////////////////////////////////////////////
  uint32_t nodeNum = 2;
  uint32_t low = 0.0;
  uint32_t high = 10.0;
  Ptr<UniformRandomVariable> rngInt = CreateObject<UniformRandomVariable> ();

  std::vector<uint32_t> shape = {nodeNum,};
  Ptr<OpenGymBoxContainer<uint32_t> > box = CreateObject<OpenGymBoxContainer<uint32_t> >(shape);

  // genera datos aleatorios
  for (uint32_t i = 0; i<nodeNum; i++){
    uint32_t value = rngInt->GetInteger(low, high);
    box->AddValue(value);
  }

  NS_LOG_UNCOND ("MyGetObservation: " << box);
  
  return box;
}

/*
Definimos funcion de recompensa
*/
float MyGetReward(void)
{
  static float reward = 0.0;
  reward += 1;
  return reward;
}

/*
Definimos informacion extra
*/
std::string MyGetExtraInfo(void)
{
  std::string myInfo = "testInfo";
  myInfo += "|123";
  NS_LOG_UNCOND("MyGetExtraInfo: " << myInfo);
  return myInfo;
}


/*
Ejecutamos acciones recibidas
*/
bool MyExecuteActions(Ptr<OpenGymDataContainer> action)
{
  Ptr<OpenGymDiscreteContainer> discrete = DynamicCast<OpenGymDiscreteContainer>(action);
  NS_LOG_UNCOND ("MyExecuteActions: " << action);
  return true;
}

void ScheduleNextStateRead(double envStepTime, Ptr<OpenGymInterface> openGym)
{
  Simulator::Schedule (Seconds(envStepTime), &ScheduleNextStateRead, envStepTime, openGym);
  openGym->NotifyCurrentState();
}

int
main (int argc, char *argv[])
{
  // Parametros del escenario
  uint32_t simSeed = 1;
  double simulationTime = 1; //Segundos
  double envStepTime = 0.1; //Segundos, ns3gym env del paso en el intertalo de segundos
  uint32_t openGymPort = 5555;
  uint32_t testArg = 0;

  CommandLine cmd;
  // Parametros requeridos para la interfaz de  OpenGym 
  cmd.AddValue ("openGymPort", "Port number for OpenGym env. Default: 5555", openGymPort);
  cmd.AddValue ("simSeed", "Seed for random generator. Default: 1", simSeed);
  // parametros opcionales
  cmd.AddValue ("simTime", "Simulation time in seconds. Default: 10s", simulationTime);
  cmd.AddValue ("testArg", "Extra simulation argument. Default: 0", testArg);
  cmd.Parse (argc, argv);

  NS_LOG_UNCOND("Ns3Env parameters:");
  NS_LOG_UNCOND("--simulationTime: " << simulationTime);
  NS_LOG_UNCOND("--openGymPort: " << openGymPort);
  NS_LOG_UNCOND("--envStepTime: " << envStepTime);
  NS_LOG_UNCOND("--seed: " << simSeed);
  NS_LOG_UNCOND("--testArg: " << testArg);

  RngSeedManager::SetSeed (1);
  RngSeedManager::SetRun (simSeed);

  // OpenGym Env
  Ptr<OpenGymInterface> openGym = CreateObject<OpenGymInterface> (openGymPort);
  openGym->SetGetActionSpaceCb( MakeCallback (&MyGetActionSpace) );
  openGym->SetGetObservationSpaceCb( MakeCallback (&MyGetObservationSpace) );
  openGym->SetGetGameOverCb( MakeCallback (&MyGetGameOver) );
  openGym->SetGetObservationCb( MakeCallback (&MyGetObservation) );
  openGym->SetGetRewardCb( MakeCallback (&MyGetReward) );
  openGym->SetGetExtraInfoCb( MakeCallback (&MyGetExtraInfo) );
  openGym->SetExecuteActionsCb( MakeCallback (&MyExecuteActions) );
  Simulator::Schedule (Seconds(0.0), &ScheduleNextStateRead, envStepTime, openGym);

  NS_LOG_UNCOND ("Simulation start");
  Simulator::Stop (Seconds (simulationTime));
  Simulator::Run ();
  NS_LOG_UNCOND ("Simulation stop");

  openGym->NotifySimulationEnd();
  Simulator::Destroy ();

}


```
### 1.3 Descripción del codigo: 

El kit de desarrollo ns3-gym consiste en dos módulos (una escrita en C++ y otra en Python) siendo complementos a los entornos de desarrollo ns-3 y OpenAI Gym que permiten el intercambio de información entre estos. La comunicación se realiza sobre sockets ZMQ1 usando la librería Protocol Buffers2 para la serialización de mensajes. Sin embargo, esto se oculta de otros usuarios dentro de una API fácil de usar. Los ambientes de simulación son definidos usando únicamente standardns-3models, mientras que los agentes pueden ser desarrollados usando librerías de machine learning como Tensorflow, Keras, entre otros.

La técnica de aprendizaje por refuerzo de define de la siguiente manera, un agente toma decisiones en un ambiente dado de forma discreta o continua, de tal manera que se induce a maximizar una noción de recompensa asociada a los estímulos y las decisiones tomadas.

Ahora, para poder comprender la inserción de inteligencia artificial a través de la herramienta OpenAI Gym se debe descomponer la técnica de aprendizaje por refuerzo en un modelo de tres componentes; el primero, es un componente de observación al cual se le integra el objeto de estudio, en este caso la red ad-hoc incluyendo la parametrización impuesta anteriormente con el fin de poder interactuar con los dos siguientes componentes del modelo de la técnica de aprendizaje por refuerzo; luego, se define el sistema de recompensas, el cual de forma iterativa asigna valores numéricos de un límite inferior a un límite superior, y se da por satisfecho al llegar al límite superior u óptimo de este sistema; y el método de cierre o conclusión de la simulación se activa cuando expira el tiempo de ejecución o se llega al límite superior del sistema de recompensas.


### 1.4 Código fuente del modelo 2 red ad-hoc con OpenAI Gym basado en el ejemplo de Linear-Mesh:

``` fichero: ./modelo_addHoc&OpenGym_2/linear-mesh-2/sim.cc```
```
#include "ns3/core-module.h"
#include "ns3/applications-module.h"
#include "ns3/opengym-module.h"
#include "ns3/mobility-module.h"
#include "ns3/wifi-module.h"
#include "ns3/internet-module.h"
#include "ns3/spectrum-module.h"
#include "ns3/stats-module.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/traffic-control-module.h"
#include "ns3/node-list.h"

#include "mygym.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("OpenGym");

int
main (int argc, char *argv[])
{
  // Parámetros del entorno
  uint32_t simSeed = 1;
  double simulationTime = 10; //seconds
  double envStepTime = 0.1; //seconds, ns3gym env step time interval
  uint32_t openGymPort = 5555;
  uint32_t testArg = 0;

  bool eventBasedEnv = true;

  //Parámetros del escenario
  uint32_t nodeNum = 25;
  double distance = 10.0;
  bool noErrors = false;
  std::string errorModelType = "ns3::NistErrorRateModel";
  bool enableFading = true;
  uint32_t pktPerSec = 1000;
  uint32_t payloadSize = 1500;
  bool enabledMinstrel = false;

  // definimos tasas de datos
  std::vector<std::string> dataRates;
  dataRates.push_back("OfdmRate1_5MbpsBW5MHz");
  dataRates.push_back("OfdmRate2_25MbpsBW5MHz");
  dataRates.push_back("OfdmRate3MbpsBW5MHz");
  dataRates.push_back("OfdmRate4_5MbpsBW5MHz");
  dataRates.push_back("OfdmRate6MbpsBW5MHz");
  dataRates.push_back("OfdmRate9MbpsBW5MHz");
  dataRates.push_back("OfdmRate12MbpsBW5MHz");
  dataRates.push_back("OfdmRate13_5MbpsBW5MHz");
  uint32_t dataRateId = 1;


  CommandLine cmd;
  // required parameters for OpenGym interface
  cmd.AddValue ("openGymPort", "Port number for OpenGym env. Default: 5555", openGymPort);
  cmd.AddValue ("simSeed", "Seed for random generator. Default: 1", simSeed);
  // parametros opcionales
  cmd.AddValue ("eventBasedEnv", "Whether steps should be event or time based. Default: true", eventBasedEnv);
  cmd.AddValue ("simTime", "Simulation time in seconds. Default: 10s", simulationTime);
  cmd.AddValue ("nodeNum", "Number of nodes. Default: 5", nodeNum);
  cmd.AddValue ("distance", "Inter node distance. Default: 10m", distance);
  cmd.AddValue ("testArg", "Extra simulation argument. Default: 0", testArg);
  cmd.Parse (argc, argv);

  NS_LOG_UNCOND("Ns3Env parameters:");
  NS_LOG_UNCOND("--simulationTime: " << simulationTime);
  NS_LOG_UNCOND("--openGymPort: " << openGymPort);
  NS_LOG_UNCOND("--envStepTime: " << envStepTime);
  NS_LOG_UNCOND("--seed: " << simSeed);
  NS_LOG_UNCOND("--distance: " << distance);
  NS_LOG_UNCOND("--testArg: " << testArg);

  if (noErrors){
    errorModelType = "ns3::NoErrorRateModel";
  }

  RngSeedManager::SetSeed (1);
  RngSeedManager::SetRun (simSeed);

  // Configuración del escenario
  // Creacion de nodos
  NodeContainer nodes;
  nodes.Create (nodeNum);

  // WiFi device
  WifiHelper wifi;
  wifi.SetStandard (WIFI_PHY_STANDARD_80211_5MHZ);

  // Channel
  SpectrumWifiPhyHelper spectrumPhy = SpectrumWifiPhyHelper::Default ();
  Ptr<MultiModelSpectrumChannel> spectrumChannel = CreateObject<MultiModelSpectrumChannel> ();

  spectrumPhy.SetChannel (spectrumChannel);
  spectrumPhy.SetErrorRateModel (errorModelType);
  spectrumPhy.Set ("Frequency", UintegerValue (5200));
  spectrumPhy.Set ("ChannelWidth", UintegerValue (5));
  
  Config::SetDefault ("ns3::WifiPhy::Frequency", UintegerValue (5200));
  Config::SetDefault ("ns3::WifiPhy::ChannelWidth", UintegerValue (5));

  // Channel
  Ptr<FriisPropagationLossModel> lossModel = CreateObject<FriisPropagationLossModel> ();
  Ptr<NakagamiPropagationLossModel> fadingModel = CreateObject<NakagamiPropagationLossModel> ();
  if (enableFading) {
    lossModel->SetNext (fadingModel);
  }
  spectrumChannel->AddPropagationLossModel (lossModel);
  Ptr<ConstantSpeedPropagationDelayModel> delayModel = CreateObject<ConstantSpeedPropagationDelayModel> ();
  spectrumChannel->SetPropagationDelayModel (delayModel);

  // añadir MAC y dataRates
  WifiMacHelper wifiMac;

  if (enabledMinstrel) {
    wifi.SetRemoteStationManager ("ns3::MinstrelWifiManager");
  } else {
    std::string dataRateStr = dataRates.at(dataRateId);
    NS_LOG_UNCOND("dataRateStr: " << dataRateStr);
    wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager",
                                  "DataMode", StringValue (dataRateStr),
                                  "ControlMode", StringValue (dataRateStr));
  }

  // Ponemos el modo ad hoc
  wifiMac.SetType ("ns3::AdhocWifiMac",
                   "QosSupported", BooleanValue (false));

  // Instalación wifi device
  NetDeviceContainer devices = wifi.Install (spectrumPhy, wifiMac, nodes);

  // Modelo de movilidad
  MobilityHelper mobility;
  mobility.SetPositionAllocator ("ns3::GridPositionAllocator",
                                 "MinX", DoubleValue (0.0),
                                 "MinY", DoubleValue (0.0),
                                 "DeltaX", DoubleValue (distance),
                                 "DeltaY", DoubleValue (distance),
                                 "GridWidth", UintegerValue (nodeNum),  // will create linear topology
                                 "LayoutType", StringValue ("RowFirst"));
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (nodes);

  // IP apilar y enrutar
  InternetStackHelper internet;
  internet.Install (nodes);

  // Asignacion de direcciones IP  a los dispositivos
  Ipv4AddressHelper ipv4;
  NS_LOG_INFO ("Assign IP Addresses");
  ipv4.SetBase ("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer interfaces = ipv4.Assign (devices);

  //Configurar el enrutamiento estático de múltiples saltos
  for (uint32_t i = 0; i < nodes.GetN()-1; i++){
    Ptr<Node> src = nodes.Get(i);
    Ptr<Node> nextHop = nodes.Get(i+1);
    Ptr<Ipv4> destIpv4 = nextHop->GetObject<Ipv4> ();
    Ipv4InterfaceAddress dest_ipv4_int_addr = destIpv4->GetAddress (1, 0);
    Ipv4Address dest_ip_addr = dest_ipv4_int_addr.GetLocal ();

    Ptr<Ipv4StaticRouting>  staticRouting = Ipv4RoutingHelper::GetRouting <Ipv4StaticRouting> (src->GetObject<Ipv4> ()->GetRoutingProtocol ());
    staticRouting->RemoveRoute(1);
    staticRouting->SetDefaultRoute(dest_ip_addr, 1, 0);
  }

  // Trafico
  // Create a BulkSendApplication and install it on node 0
  Ptr<UniformRandomVariable> startTimeRng = CreateObject<UniformRandomVariable> ();
  startTimeRng->SetAttribute ("Min", DoubleValue (0.0));
  startTimeRng->SetAttribute ("Max", DoubleValue (1.0));

  uint16_t port = 1000;
  uint32_t srcNodeId = 0;
  uint32_t destNodeId = nodes.GetN() - 1;
  Ptr<Node> srcNode = nodes.Get(srcNodeId);
  Ptr<Node> dstNode = nodes.Get(destNodeId);

  Ptr<Ipv4> destIpv4 = dstNode->GetObject<Ipv4> ();
  Ipv4InterfaceAddress dest_ipv4_int_addr = destIpv4->GetAddress (1, 0);
  Ipv4Address dest_ip_addr = dest_ipv4_int_addr.GetLocal ();

  InetSocketAddress destAddress (dest_ip_addr, port);
  destAddress.SetTos (0x70); //AC_BE
  UdpClientHelper source (destAddress);
  source.SetAttribute ("MaxPackets", UintegerValue (pktPerSec * simulationTime));
  source.SetAttribute ("PacketSize", UintegerValue (payloadSize));
  Time interPacketInterval = Seconds (1.0/pktPerSec);
  source.SetAttribute ("Interval", TimeValue (interPacketInterval)); //packets/s

  ApplicationContainer sourceApps = source.Install (srcNode);
  sourceApps.Start (Seconds (0.0));
  sourceApps.Stop (Seconds (simulationTime));

  // Cree un receptor de paquetes para recibir estos paquetes
  UdpServerHelper sink (port);
  ApplicationContainer sinkApps = sink.Install (dstNode);
  sinkApps.Start (Seconds (0.0));
  sinkApps.Stop (Seconds (simulationTime));

  // Imprimir posiciones de nodo
  NS_LOG_UNCOND ("Node Positions:");
  for (uint32_t i = 0; i < nodes.GetN(); i++)
  {
    Ptr<Node> node = nodes.Get(i);
    Ptr<MobilityModel> mobility = node->GetObject<MobilityModel> ();
    NS_LOG_UNCOND ("---Node ID: " << node->GetId() << " Positions: " << mobility->GetPosition());
  }

  // OpenGym Env
  Ptr<OpenGymInterface> openGymInterface = CreateObject<OpenGymInterface> (openGymPort);
  Ptr<MyGymEnv> myGymEnv;
  if (eventBasedEnv)
  {
    myGymEnv = CreateObject<MyGymEnv> ();
  } else {
    myGymEnv = CreateObject<MyGymEnv> (Seconds(envStepTime));
  }
  myGymEnv->SetOpenGymInterface(openGymInterface);

  // connect OpenGym entity to event source
  Ptr<UdpServer> udpServer = DynamicCast<UdpServer>(sinkApps.Get(0));
  if (eventBasedEnv)
  {
    udpServer->TraceConnectWithoutContext ("Rx", MakeBoundCallback (&MyGymEnv::NotifyPktRxEvent, myGymEnv, dstNode));
  } else {
    udpServer->TraceConnectWithoutContext ("Rx", MakeBoundCallback (&MyGymEnv::CountRxPkts, myGymEnv, dstNode));
  }

  NS_LOG_UNCOND ("Simulation start");
  Simulator::Stop (Seconds (simulationTime));
  Simulator::Run ();
  NS_LOG_UNCOND ("Simulation stop");

  myGymEnv->NotifySimulationEnd();
  Simulator::Destroy ();
}
```


### 2. Manual técnico:

### Requerimientos: 

- Ubuntu 19.10
- NS3
- OpenAI Gym

### Escenario 1:
1. Para correr el ejemplo simple de NS3 se hace con el siguiente comando con el archivo dentro de la carpeta scratch parados en la terminal desde la raiz de el programa de NS3

```./waf --run scratch/modelo_AddHoc_OnlyNs3/red-ad-hoc --vis```
### Escenario 2:
2. Para correr el ejemplo con OpenAI Gym se hace con el siguiente comando donde se debe tener el archivo simple_test.py y sim.cc en su carpeta aislada y sin otros ficheros dentro dentro de la carpeta scratch y parados en la terminal desde donde esta el archivo ./modelo_addHoc&OpenGym_1/simple_test.py.

```./modelo_addHoc&OpenGym_1/simple_test.py```

### Escenario 3:
3. 
a. Para correr el ejemplo con OpenAI Gym se hace con el siguiente comando donde se debe tener el archivo simple_test.py y sim.cc en su carpeta aislada y sin otros ficheros dentro dentro de la carpeta scratch y parados en la terminal desde donde esta el archivo ./modelo_addHoc&OpenGym_2/linear-mesh-2/simple_test.py.

```./modelo_addHoc&OpenGym_2/simple_test.py```
b. Para correr el ejemplo con OpenAI Gym ahora con debug de el comportamiento de los nodos se hace siguiendo los mismos parametros anteriores pero ahora desplegando en 2 terminales de la siguiente forma.
```
# Terminal 1
./waf --run "linear-mesh-2"

# Terminal 2
cd ./scratch/linear-mesh-2/
./simple_test.py --start=0
```

### Herramientas utilizadas en el desarrollo: 

#### NS3: 

Este es un framework el cual está dedicado exclusivamente a la aplicación de técnicas de inteligencia artificial, cuyo objetivo es estandarizar las rutinas aplicadas por la comunidad con esto se pueden replicar experimentos y acelerar los avances en el campo de la inteligencia artificial.

#### OpenAI Gym:

Este software se encarga principalmente de simular redes de computadores, cuya extensión aplica principalmente al estudio de redes móviles ad-hoc al permitir la implementación de protocolos de redes cableadas y de redes inalámbricas.

## 3. Resultados.
1. Inicialmente tenemos una red de 25 nodos conectados en una red Ad-Hoc
![NS1](/img/ns1.jpg)
2. Una vez se inicia la simulación se empiezan a mandar datos 100kps cada 3 segundos entre los nodos más cercanos
![NS2](/img/ns2.jpg)
3. Cada vez los nodos se van moviendo aleatoriamente y formándose grupos
![NS3](/img/ns3.jpg)
4. Y progresivamente se siguen enviando datos y se forman grupos donde se envían datos hacia sus compañeros más cercanos
![NS4](/img/ns4.jpg)
5. Hasta que unos nodos se aíslan completamente y otros se comunican solo con sus cercanos
![NS5](/img/ns5.jpg)
6. Ya aplicado a OpenAI Gym la idea es variar los parámetros de movimiento de los nodos de la red Ad-Hoc de tal forma que el objetivo sea un movimiento que permita tener la red transmitiendo la mayor cantidad de datos entre los nodos donde se ve el escenario ideal y es que todos los nodos se vayan en una misma dirección transmitiendo la misma cantidad de datos entre todos los nodos de manera constante.
![NS6](/img/ns6.jpg)
7. Después de aplicar la técnica de aprendizaje en promedio en el paso 10, la red Ad-Hoc empieza a generar un patrón el cual empieza a balancear las distancias entre nodos y su conectividad de tal manera que maximice el área de cobertura sin desconectar nodos de la red, sin embargo, este no es capaz de forzar a todos los nodos para que sean parte de una sola red ad-hoc, es decir, no evita la creación de subredes que no tienen forma de conectarse con otras redes.
![NS7](/img/ns7.jpg)

| ------------- | 
| Puntos de prueba| 
| ------------- | 
| 57  | 
| 86  | 

8. Ahora, basados en el ejemplo sobre tipología meshnet la cual conecta directamente, de forma dinámica y sin jerarquía a todos los nodos los cuales sea eficiente una conexión, dando como consecuencia la habilidad de auto-configurarse y auto-organizarse. De forma intuitiva se puede evidenciar que el uso de esta tipología a la red ad-hoc y a su vez establecer la técnica de aprendizaje por refuerzo se concluye que la red ad-hoc tiene los suficientes insumos para poder maximizar el área de cobertura y a su vez las conexiones de forma efectiva y eficiente dentro de una única red.

## 4. Referencias.
- [Introduction: Reinforcement Learning with OpenAI Gym](https://towardsdatascience.com/reinforcement-learning-with-openai-d445c2c687d2) 
- [ns3-gym: Extending OpenAI Gym for Networking Research](https://arxiv.org/pdf/1810.03943.pdf)
- [ns-3 meets OpenAI Gym: The Playground for Machine Learning in Networking Research](http://www.tkn.tu-berlin.de/fileadmin/fg112/Papers/2019/gawlowicz19_mswim.pdf)
