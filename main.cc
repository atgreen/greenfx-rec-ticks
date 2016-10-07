// --------------------------------------------------------------------------
//  _____                    ________   __
// |  __ \                   |  ___\ \ / /
// | |  \/_ __ ___  ___ _ __ | |_   \ V /          Open Source Tools for
// | | __| '__/ _ \/ _ \ '_ \|  _|  /   \            Automated Algorithmic
// | |_\ \ | |  __/  __/ | | | |   / /^\ \             Currency Trading
//  \____/_|  \___|\___|_| |_\_|   \/   \/
//
// --------------------------------------------------------------------------

// Copyright (C) 2014, 2106  Anthony Green <anthony@atgreen.org>
// Distrubuted under the terms of the GPL v3 or later.

// This progam pulls ticks from the A-MQ message bus and records them
// to disk.

#include <cstdlib>
#include <memory>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>

#include <decaf/lang/Thread.h>
#include <decaf/lang/Runnable.h>

#include <activemq/library/ActiveMQCPP.h>
#include <activemq/core/ActiveMQConnectionFactory.h>
#include <cms/ExceptionListener.h>
#include <cms/MessageListener.h>

#include <json/json.h>

using namespace decaf::util::concurrent;
using namespace decaf::util;
using namespace decaf::lang;
using namespace activemq;
using namespace cms;
using namespace std;

#include "config.h"
 
class TickListener : public ExceptionListener,
		     public MessageListener,
  		     public Runnable {

private:
  Session *session;
  Connection *connection;
  Destination *destination;
  MessageConsumer *consumer;
  FILE *ticklog_f;

  string brokerURI;

public:
  TickListener () :
    brokerURI("tcp://broker-amq-tcp:61616?wireFormat=openwire") {
    ticklog_f = fopen ("/var/rec-ticks/ticklog.csv", "a+");
  }

  virtual void run () {
    try {

      // Create a ConnectionFactory
      std::auto_ptr<ConnectionFactory> 
	connectionFactory(ConnectionFactory::createCMSConnectionFactory(brokerURI));
      
      // Create a Connection
      connection = connectionFactory->createConnection("user", "password");
      connection->start();
      connection->setExceptionListener(this);
      
      session = connection->createSession(Session::AUTO_ACKNOWLEDGE);
      destination = session->createTopic("OANDA.TICK");
      
      consumer = session->createConsumer(destination);
      consumer->setMessageListener(this);

      // Sleep forever
      while (true)
	sleep(1000);
      
    } catch (CMSException& ex) {
      
      printf (ex.getStackTraceString().c_str());
      exit (1);
      
    }
  }

  virtual void onMessage(const Message *msg)
  {
    json_object *jobj = json_tokener_parse (dynamic_cast<const TextMessage*>(msg)->getText().c_str());

    jobj = json_object_object_get(jobj, "tick");
    if (jobj != NULL)
      {
	json_object *bid = json_object_object_get(jobj, "bid");
	json_object *ask = json_object_object_get(jobj, "ask");
	json_object *instrument = json_object_object_get(jobj, "instrument");
	json_object *time = json_object_object_get(jobj, "time");
	fprintf (ticklog_f, "%s,%s,%s,%s", 
		 json_object_get_string (instrument),
		 json_object_get_string (time),
		 json_object_get_string (bid),
		 json_object_get_string (ask));
      }
  }

  virtual void onException(const CMSException& ex)
  {
    printf (ex.getStackTraceString().c_str());
    exit (1);
  }
};

int main()
{
  printf ("Program started by User %d", getuid());

  activemq::library::ActiveMQCPP::initializeLibrary();

  TickListener tick_listener;
  Thread listener_thread(&tick_listener);
  listener_thread.start();
  listener_thread.join();

  printf ("Program ended");

  return 0;
}

