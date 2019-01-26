# internet-of-fairy-lights
An experiment with Particle Mesh to make internet-connected string LEDs that operate in unison.

## What is a fairy light?

Fairy lights are also known as string LEDs. They consist of two wires with LEDs connected every few inches. The ones I am using alternate the polarity of the LEDs so that by powering one lead and grounding the other half the LEDs will light up, reversing the power will light the other half of the LEDs. This makes them useful for creating interesting lighting displays in everything from dorm rooms to Christmas trees.

## Goals

- Develop a system of controllers for fairy lights that allows them to operate in unison and be controlled remotely
- Develop a cloud backend that can communicate with the fairy light controllers
- Develop Alexa and Google Assistant integrations to control the fairy lights remotely

## What am I using?

For the controllers, I am using one Particle Argon Wifi + Bluetooth + Mesh Kit and two Particle Xenon Bluetooth + Mesh kits. Both are available on [Adafruit](https://www.adafruit.com/?q=particle%20mesh) as well as on [Particle's website](https://www.particle.io/mesh). I am also using [these fairy lights](https://smile.amazon.com/gp/product/B07DV4LXJP/ref=ppx_yo_dt_b_asin_title_o00__o00_s00?ie=UTF8&psc=1). I am still deciding what cloud provider to use for the backend.