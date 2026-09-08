# AppIso Tutorial

AppIso generates a 3D terrain from a set of **isolines** (contour lines), and lets you edit those isolines by hand. The general workflow is:

1. Pick a **mask** (the coastline / land shape) and a **histogram** (the distribution of elevations you want).
2. Click **Generation** to automatically produce a set of isolines that match the mask and histogram.
3. Optionally hand-edit the isolines (draw new ones, smooth, warp, slope, protect zones from later edits).
4. The 3D terrain and the render on the right update live from the isolines.
5. Export the result as images, an SVG of the isolines, or a photorealistic (PBR) render.

The window is split into three areas:

- **Left panel** — the "Iso Actions" tab: generation parameters, editing tools, and export buttons.
- **Center panel** — the live 3D terrain view
- **Right panel** — Camera and terrain controls.
- **Bottom panel ("IsoView")** — the 2D top-down view of the isolines themselves, where you draw and edit them.

---

## 1. Generation panel (left, top section)

### Paramètres (mask / noise / histogram inputs)

| Control | What it does |
|---|---|
| **Mask** button | Opens a file browser to load a black & white (or grayscale) image as the generation mask. White/bright areas become land, dark areas become sea. The small preview to its left shows the current mask, filled with its palette. |
| **Nb isos** (next to Mask) | Number of isolines to derive directly from the mask's own shape when you use **Reset / Mask** (see below) — i.e. how many concentric contour rings to trace inward from the coastline. |
| **Sea level** | The threshold, on the mask's [0,1] range, used to distinguish sea from land, and used as the baseline height for generation. |
| **Noise** button | Opens a file browser to load a grayscale image used as an additional height perturbation ("Noise") layered on top of the generated terrain, for natural irregularity. Small preview shown next to Mask. |
| **Pow** (next to Noise) | Exponent applied to the noise image before it's used — higher values push the noise contrast/intensity up. |
| **Histogram** button | Opens a file browser to load a **heightfield** (grayscale DEM image) whose elevation distribution is analyzed and reused as the target elevation histogram for generation. The small chart preview on the right shows the resulting histogram (bar chart). |
| **Nb bins** | Number of bins used when building the histogram from the loaded file. |
| **File / function dropdown** | Instead of loading a histogram from a file, you can pick an analytic shaping function (`x`, `1 - x`, `exp(x)`, `exp(1-x)`, `sin(πx)`, `sin(π(1+x))`, or a flat `1`) to synthesize the target elevation distribution instead. |
| **N particles** | The desired number of sample points ("particles") for the Poisson-disc point distribution that seeds the terrain's zone graph. The app back-computes the sampling radius from this number. |
| **View Mask** | This turns whatever isolines are currently drawn in IsoView *into* the new generation mask. Workflow: **Clean View**, hand-draw a shape with the Tracing tool, then click **View Mask** to use that shape as the mask for the next generation. If you have one isoline drawn, you get a one-region mask from it, and so on. |
| **(km) cX / cY / W / H** | The generation box: the center (cX, cY) and size (W × H, in kilometers) of the area that will actually be generated. |

### Generation

- **Generation** button — runs the full pipeline: builds the zone graph from the mask (Poisson-disc sampling), assigns elevations to each zone (an Eden-growth-style simulation), and extracts isolines from the resulting elevation field matched against your histogram. This is the main "go" button.
- Next to it, three read-outs update after each run: number of particles used, time taken (ms), and number of isoline points produced.

---

## 2. Edition panel (left, middle section)

### Box/Terrain

| Control | What it does |
|---|---|
| **(km) X / Y / W / H** | The view/edit box: which region of the isolines is shown and editable in the IsoView panel below. |
| **(m) Min / Max** | The minimum and maximum elevation (in meters) that the current isolines are rescaled to. |
| **(m) X / Y** (terrain size) | Resolution (in samples) of the reconstructed heightfield, i.e. how finely the terrain grid is sampled. |
| **Stairs / Interpolate / Smooth stairs / Eroded** (radio buttons) | How the isolines are turned into an actual heightfield: <br>• **Stairs** — a stepped terrain, flat between each isoline (literal terraces). <br>• **Interpolate** — a smooth, continuously interpolated surface between isoline heights. <br>• **Smooth stairs** — the stepped version with the steps smoothed/rounded off. <br>• **Eroded** — the interpolated surface with an extra erosion simulation pass applied on top. <br>⚠️ **Eroded runs 3 erosion simulation passes on the whole heightfield, and the erosion cost/risk grows with terrain resolution.** Keep the **(m) X / Y** terrain-size fields above at or below 256 before selecting Eroded — using it on a larger terrain can make the app hang or crash. |

### Tools row

- **Tracing / Smoothing / Warping / Sloping / Protecting** (dropdown) — selects the active editing tool for the IsoView panel (right-click-drag there to use it; see section 4 below for exactly what each tool does).
- **Clean View** — clears all isolines from the current view (starts empty).
- **Reset Zoom** — resets IsoView's pan/zoom back to fit the current box.
- **Reset / Mask** — discards current isolines and regenerates a fresh set directly from the mask's own outline (using **Nb isos** concentric rings), ignoring the histogram/particle pipeline. ⚠️ This button is known to be a bit buggy — if the result looks wrong, try again or fall back to the normal **Generation** pipeline.

### Options (changes with the selected tool)

- **Tracing**: no extra options — right-click-drag on IsoView to draw a new closed isoline. ⚠️ The drawn shape **cannot cross itself or any other existing isoline** — if it does, it is silently rejected and nothing is added (no error message, the line just disappears when you release the mouse). Draw a simple, non-intersecting closed loop.
- **Smoothing**: **Dist max (m)** — the smoothing distance (how far along the curve the smoothing kernel reaches); **Neighs weight** — smoothing strength/weight between 0 and 1 for how much neighboring points influence the smoothing.
- **Warping**: **Strength μ**, **Factor ε**, **Power n** — control the displacement warp applied along the drawn path: μ is the overall displacement strength, ε how far the effect reaches, n shapes the falloff curve.
- **Sloping**: **Strength μ**, **Power n** — same idea as warping, but the isolines are displaced according to a slope-shaping function instead.
- **Protecting**: marks circular zones that are meant to stay untouched by the other editing tools (Smoothing, Sloping, drawing over them, etc.) — a way to lock down parts of your isolines before making a broader edit elsewhere. ⚠️ This protection is **not respected by the Warping tool** — isolines inside a protected zone still move when you warp. **Removing circles** — when checked, dragging on IsoView *removes* protected zones instead of adding them; **Showing zones** — toggles a visual overlay of the currently protected circles; **Protect small isos** + its numeric field — automatically protects every isoline shorter than the given length (a quick way to lock small islands/details before doing a broad edit); **Clean zones** — removes all protected zones.

> For the precise mathematical meaning of the Smoothing/Warping/Sloping parameters (μ, ε, n, etc.), see the research paper this application accompanies.

### Presets and helper buttons

- **Hawaii / Reunion / Man** — load one of three built-in preset coastline polygons as the current isolines (quick way to try the app without your own data — "Man" is the Isle of Man, not a humanoid outline).
- **Min dist** / **Resample** / **Res. Spline** — **Min dist** sets the target minimum spacing between isoline points; **Resample** redistributes points along every isoline to roughly that spacing (uniform polyline resampling); **Res. Spline** does the same but fits a smooth spline through the points first, so the result is a curve rather than straight segments. The "> N pts" readout shows the expected point count after resampling.
- **(m) Radius** — the brush radius (in meters) used by the Smoothing / Warping / Sloping / Protecting tools; shown as a red circle following your cursor in IsoView while one of those tools is active. You can also change it live with **Alt + mouse wheel** while hovering IsoView.

---

## 3. IsoView (bottom panel)

This is the top-down 2D editor for the isolines.

- **Left-click + drag** — pan the view.
- **Mouse wheel** — scroll vertically.
- **Shift + wheel** — scroll horizontally.
- **Ctrl + wheel** — zoom in/out (centered on the cursor).
- **Alt + wheel** — change the current tool's brush radius (the red circle).
- **Right-click + drag** — perform the action of the currently selected tool (see above): draw a new isoline, smooth/warp/slope existing ones, or protect a zone.
- **Left-click on an isoline** — select it (selected isolines highlight and can be deleted).
- **Delete** — removes the currently selected isoline(s).
- **Ctrl+Z / Ctrl+Y** — undo / redo (isoline edit history; there is no dedicated Undo/Redo button, only these shortcuts).

Isolines are colored by elevation using a blue → white → red palette (low to high).

---

## 4. 3D view and Camera (center)

The top-right panel is a live, shaded (wireframe/mesh-style) preview of the generated terrain — it updates automatically whenever the isolines or generation parameters change.

**Mouse controls inside the 3D view** (hold **Alt**, then):
- **Alt + Left-drag** — orbit the camera around its look-at point.
- **Alt + Right-drag** — dolly the camera forward/backward.
- **Alt + Middle-drag** — pan the camera (and its look-at point) sideways.
- **Mouse wheel** (no modifier) — zoom in/out.

**Keyboard shortcuts inside the 3D view:**
- **F1** — save a quick 720p screenshot of the 3D view to the app folder.
- **F2** — save a 4K screenshot.
- **F3** — hot-reload the GLSL shaders from disk (useful only if you're editing the shader files yourself).
- **Alt+S** — toggle a performance/statistics overlay.
- **Alt+C** — toggle a camera-info overlay.

**Camera panel** (right of the 3D view):
- **Reset Camera** — restores the default orbit view.
- **LookDown Camera** — switches to a straight top-down view.
- **Longitude / Latitude / Dist** — the camera's orbit position in spherical coordinates around its look-at point (angle, angle, distance).
- **At x / y / z** — the world-space point the camera is looking at / orbiting around.

Changing any of these fields (or moving the mouse in the 3D view) updates the others to match.

---

## 5. Actions / Export (left, bottom section)

- **Export stairs / Export smooth stairs / Export interpolate** — save the terrain reconstructed with that specific method (see the radio buttons in section 2) as a grayscale heightfield image, regardless of which radio button is currently selected in the UI.
- **Export PBR** — runs the erosion simulation (MSE) and renders the result with the photorealistic (path-traced) shader, then saves it as an image.
- **Export isolines** — exports a clean SVG redraw of the current isolines (using the export-specific display options, independent of what's currently shown on screen).
- **Export scene** — exports the IsoView panel exactly as currently displayed (including anything like protected-zone overlays) as an SVG file.
- **Export All** — runs every export that's checked below (see the checkboxes: **isos**, **pbr**, **stairs**, **smooth**, **inter**) in one go, writing all of them into the export folder without prompting.
- **Figures/** (text field) — the output folder/prefix used by **Export All** only. Change this to redirect that batch export elsewhere.
- The individual export buttons (Export stairs/smooth stairs/interpolate/PBR/isolines/scene) each open a native **Save File** dialog for you to pick the destination and filename — they don't use the "Figures/" field. Only **Export All** writes directly, using "Figures/" as its prefix.
- The five buttons at the top (Export stairs/smooth stairs/interpolate/PBR/isolines) are disabled until there is at least one isoline in the current view.

> **Generate Figures** (top-right, above the Camera panel) is a separate, research-specific feature: it batch-regenerates the exact numbered figures used in the paper this application accompanies, and isn't intended for general use.

---

## 6. Misc Process (right section)

- **Scale factor field + Scale** — multiplies the current heightfield's elevation by the given factor (vertical exaggeration/flattening).
- **Smooth** — applies a smoothing pass directly to the current heightfield (independent of the isoline-smoothing tool described above).

---

## Notes

- Qt console/debug output (particle counts, warnings, render progress, etc.) is printed to the console window that runs alongside the app; keep it open if you want to see generation logs or errors.
