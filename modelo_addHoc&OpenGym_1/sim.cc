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
