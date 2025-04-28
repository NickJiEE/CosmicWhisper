# Cosmic Whisper
**Cosmic Whisper** is an educational puzzle game that challenges players and demonstrates knowledge of **optics and circuits** in solving a mystery to save the world. Players take on the role of scientists attempting to establish communication with an alien civilization orbiting a chaotic three-star system, drawing inspiration from the _Three-Body Problem_. To achieve this, they must overcome scientific challenges and successfully transmit a signal, which involves with **light absorption and transmission**, **spectroscopy**, **optical encoding/deciphering (Morse code & Vigenère cipher)**, and **light intensity modulation** to create an immersive learning experience, manipulating **light**, **sound**, and **electrical circuits** to complete each challenge. Through hands-on gameplay, **Cosmic Whisper** provides an engaging way to explore fundamental concepts in physics and electrical engineering. Developed as a **research project at UC San Diego under the supervision of Dr. Saharnaz**, the game blends scientific principles with interactive problem-solving.

## Materials Required
The game is built using a combination of **3D-printed components** and **electronic hardware**, including:
- **Optical Components:** Diffraction grating slide, light source (flashlight/light bulb), glass/plastic containers, laser pointers
- **Electronics & Sensors:** ESP32-CAM, ESP32, LEDs, linear actuator, relay module, IR LED, IR sensor, IR remote, LED strips, number pad, servo motor

## Game Logistics
Players must solve a series of **three interconnected puzzles** to complete their mission:

1. **Spectroscopy Challenge:** Identify the correct substances among a set of samples using a spectrometer and UV light. The correct substances reveal a password and keyword, allowing access to the next stage.
2. **Morse Code Decryption:** Observe the blinking LED sequences and decode the Morse code, decipher the Vigenère cipher to unlock the second password.
3. **Signal Transmission Puzzle:** Align the signal tower (a laser pointer mounted on a servo motor) to the sun using the control panel, setting the correct coordinates. When the LED strips light up, it confirms the signal has been successfully transmitted, completing the game.

## Substances Chart
<p align="center">
  <img src="images/substances_chart.png" alt="substances setup">
</p>

**Substances set up:** For food dyes, 1 drop of each into 50 mL vials.

## Understanding Spectroscopy and Food Dye Absorption
### How the Spectrometer Works
A spectrometer is an optical instrument that analyzes the composition of light by breaking it down into its **wavelength components**. It works by:
1. Passing light through a sample (in this case, food dye solutions).
2. Using a diffraction grating or prism to separate the light into its spectrum.
3. Detecting the intensity of each wavelength and plotting it as a graph.

When light shines through a **colored liquid**, certain wavelengths are **absorbed** by the substance while others **pass through** or are **transmitted**. The spectrometer captures these patterns, producing a **spectral absorption graph**, like the ones in the images below.

<p align="center">
  <img src="images/set2.png" alt="set2">
</p>

<p align="center">
  <img src="images/set3.png" alt="set3">
</p>

### Interpreting the Spectral Plots
Each plot shows **intensity vs. wavelength**, revealing which colors of light are absorbed or transmitted:
- Sharp peaks indicate **high transmission** (less absorption) of that wavelength
- Dips or missing wavelengths correspond to **absorption**, meaning the dye absorbs those colors from the light

For example, in the above graphs: 
- The first graph (Set 2) has a strong peak in the **blue/violet region** (~450 nm), suggesting that one of the substances absorbs less blue light and transmits it more
- The second graph (Set 3) has broader peaks in the **red** and **yellow regions** (~600–650 nm), yet only one has a peak at **blue/violet region** (~450 nm), suggesting that only one substance shows obvious **blue/violet** on the spectrum

By analyzing these spectra visually on the ESP32-CAM, players in the game can determine **which substances contain specific wavelengths** and use that information to unlock the puzzle answers.

## Components Designed
Case, spectrometer, etc...
