Insteon Compatible WiFi Hub
===============
> Open Source Secure WiFi Hub for Insteon Devices
***

The Hub I is designed to provide secure cloud control to your existing Insteon network. We are currently working on prototyping with the Spark Core. The final solutions will use the Spark Photon P1.

## Feedback & Support

Please let us know if you like this concept.  We need your support and encouragement to make the prototype a product. Send us an email, [support@automategreen.com](mailto:support@automategreen.com), or a tweet, [@AutomateGreen](https://twitter.com/AutomateGreen).

## Work In Progress

Link Record DB - The all-link record database is not implement yet.  Linking will not work until this is complete.

## UPDATE - CRC Solved!!!

The Insteon CRC issue has been solved.  Thank you Wieslaw Strugala for your help.  Insteon uses two different CRC methods: one for power line (PL) and one for radio (RF).  Wieslaw has solved the PL CRC and pointed me to Peter Shipley who solved the Insteon RF CRC, <https://github.com/evilpete/insteonrf>.  Many thanks to Peter, Wieslaw, and all the others who helped.

## Insteon Over the Spark Cloud

To improve on the standard Insteon Hub, we've used the awesome [Spark OS](https://www.spark.io/) to extend the PLM interface security to the cloud.  No unencrypted open information here. All messaging pass securely to and from the Spark OS.

### Insteon Message

All Insteon messages are sent as HEX strings.  The HEX strings allow for all Insteon messages to fit within the Spark OS packet restriction.  It also minimizes the logic for the hub.

Because the HEX is cryptic and requires an in-depth knowledge of the Insteon HEX commands, we added support for the hub to our Node.js module, [home-controller](https://github.com/automategreen/home-controller).

### Sending an Insteon Command

All the Insteon commands can be sent using HEX strings.

For example, to turn an Insteon switch on, you send a direct command to turn on the device.

```
$ spark call 0123456789ABCDEFGHI insteon 0262AABBCC0F117F
8
```

- Direct command: 0262
- Device ID: AABBCC
- Flags: 0F
- Action (turn on): 11
- Level (50%): 7F


The light turns on to 50%, and the call returns the number of bytes sent to the PLM. In this case `8`. 


### Receiving an Insteon Event

Now you can control your device, but you want more.  You want notifications in real time of from you devices - no problem.  You can use the Spark OS subscribe to receive Insteon messages from your device.

Here's and example of getting notified of motion detected by a motion sensor.

```
$ spark subscribe insteon 0123456789ABCDEFGHI
Subscribing to "insteon" from 0123456789ABCDEFGHI's stream
Listening to: /v1/devices/0123456789ABCDEFGHI/events/insteon
{"name":"insteon","data":"0250aabbcc000001cf1101","ttl":"60","published_at":"2015-01-12T21:16:48.361Z","coreid":"0123456789ABCDEFGHI"}
```

The data field contains the Insteon message.

- Received command: 0250
- Device ID: AABBCC
- Group: 000001
- Flags: cf
- Action (motion detected): 11

### Insteon Request/Response

If you want to query a device for information, you combine the `call` and `subscribe` together. 

First, make sure you subscribe.

```
spark subscribe insteon 0123456789ABCDEFGHI
Subscribing to "insteon" from 0123456789ABCDEFGHI's stream
Listening to: /v1/devices/0123456789ABCDEFGHI/events/insteon
```

Then, send the request. For example, get the light level.

```
$ spark call 0123456789ABCDEFGHI insteon 0262aabbcc0f1900
2
```

You'll then receive the Insteon response from the subscription.

```
{"name":"insteon","data":"0250aabbccffffff2f01ff","ttl":"60","published_at":"2015-01-12T21:16:48.361Z","coreid":"0123456789ABCDEFGHI"}
```

Light is on at 100%.  

If your want an easier way, you can use [home-controller](https://github.com/automategreen/home-controller).