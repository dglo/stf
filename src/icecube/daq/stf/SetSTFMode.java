package icecube.daq.stf;

import icecube.daq.domhub.common.*;

import java.rmi.Naming;
import java.util.Iterator;

public class SetSTFMode {
   public static void main(String [] args) {
      if (args.length!=1) {
	 System.out.println("usage: SetSTFMode domhub_host");
	 return;
      }

      String rmiHost = args[0];
      String dhUrl = "rmi://" + rmiHost + "/domhubapp";
      
      try {
	 DOMHubCom dhc = (DOMHubCom)Naming.lookup(dhUrl);

	 /* assume that, if there are no doms, we
	  * need to power on...
	  */
	 if (dhc.getDOMStatusList().getDOMCount()==0) {
	    dhc.powerUpAllChannels();
	 }
	 
	 DOMStatusList dsl = dhc.getDOMStatusList();
	 
	 for (int i = 0; i < dsl.getDOMCount(); i++) {
	    DOMStatus ds = dsl.getDOMStatus(i);

	    /* System.out.println(ds); */

	    ds = dhc.reserveDOM(ds.getDOMID(),
				DOMReservations.STF_CLIENT);


	    if (ds.getDOMRunState()!=DOMRunStates.STF_SERVER) {
	       ds =
		  dhc.chgDOMRunState(ds.getDOMID(),
				     DOMReservations.STF_CLIENT,
				     DOMRunStates.STF_SERVER);

	    }

	    dhc.closeServerChannel(ds.getDOMID(),
				    DOMReservations.STF_CLIENT,
				    ChannelTypes.SERIAL_COM_SESSION);
	    
	    DOMSocketChannel dsc = 
	       dhc.getServerChannel(ds.getDOMID(),
				    DOMReservations.STF_CLIENT,
				    ChannelTypes.SERIAL_COM_SESSION);

	    System.out.println(dsc.getHost() + "\t" +
			       (dsc.getPort()-3000) + "\t" +
			       ds.getCard() + "\t" +
			       ds.getChannel() + "\t" +
			       ds.getSubchannel());
	 }
      } catch (Exception e) {
	 e.printStackTrace(System.out);
      }
   }
}


