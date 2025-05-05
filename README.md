# Venipuncture_trainer
 This is a device for training of venipuncture. LEDs and a Buzzer indicate failure in case of  overpenetrating the vessel.


# Open-Source Electronic Venipuncture Trainer

> Real-time error feedback for safer, smarter practice  
> **SimLab · UKE Hamburg** · CC-BY-SA 4.0 / MIT

![Hero image](1st_render.png)

![Hero Video](test_needle.mp4)


---

## 1 · What is it?

A fully 3-D-printable training pad (silicone + conductive layers) with Arduino-based
resistance sensing. If the needle overshoots the vein, the system triggers a
**short-short-short-long** LED/Buzzer alert.

| Feature | Why it matters |
|---------|----------------|
| **Carbon-silicone layers** | Realistic feel, corrosion-free |
| **Pressurised veins** | Heat-shrink tubing fed by a water reservoir |
| **Instant Arduino feedback** | Audible + visual—no hidden errors |
| **Python live monitor** | Streams raw resistance values to your PC |
| **100 % open source** | Remix, improve, teach—no strings attached |

---

## 2 · Quick Start

```bash
git clone https://github.com/jamballama/Venipuncture_trainer
cd venipuncture-trainer
