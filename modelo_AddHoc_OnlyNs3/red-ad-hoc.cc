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
NS_LOG_COMPONENT_DEFINE ("RedAddHoc");


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
