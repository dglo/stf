package icecube.daq.stf;

import icecube.daq.domhub.common.*;

import java.rmi.Naming;
import java.util.Iterator;

public class AllOff {
   public static void main(String [] args) {
      if (args.length!=1) {
	 System.out.println("usage: AllOff domhub_host");
	 return;
      }

      String rmiHost = args[0];
      String dhUrl = "rmi://" + rmiHost + "/domhubapp";
      
      try {
	 DOMHubCom dhc = (DOMHubCom)Naming.lookup(dhUrl);

	 if (dhc.getDOMStatusList().getDOMCount()>0) {
	    dhc.powerOffAllChannels();
	 }
      } catch (Exception e) {
	 e.printStackTrace(System.out);
      }
   }
}
