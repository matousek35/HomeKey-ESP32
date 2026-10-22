<script lang="ts">
  import type { MiscConfig } from "$lib/types/api";
  let { config = $bindable() }: { config: MiscConfig } = $props();
</script>

<div class="space-y-5">
  <p class="text-sm text-base-content/70">Settings are applied after saving and restarting the device.</p>
  <section class="p-3 bg-base-100 rounded-lg space-y-3">
    <h3 class="font-semibold">Doorbell button</h3>
    <label class="flex items-center gap-3"><input type="checkbox" class="toggle toggle-primary toggle-sm" bind:checked={config.doorbellEnabled} />Enable GPIO doorbell</label>
    <label class="form-control block">Button GPIO
      <input type="number" min="0" max="48" step="1" required class="input input-sm input-bordered w-full" bind:value={config.doorbellGpio} />
    </label>
    <p class="text-xs text-base-content/60">Connect a normally open, dry-contact button between this GPIO and GND. Internal pull-up; active low. This switch does not disable the separate keypad doorbell.</p>
  </section>
  <section class="p-3 bg-base-100 rounded-lg space-y-3">
    <h3 class="font-semibold">OV5640 camera</h3>
    <label class="flex items-center gap-3"><input type="checkbox" class="toggle toggle-primary toggle-sm" bind:checked={config.cameraEnabled} />Enable camera</label>
    <p class="text-xs text-base-content/60">Requires the camera firmware variant and PSRAM. The Waveshare camera connector has fixed wiring: change pins only if your physical wiring changes. Power enable is active low; PWDN and RESET are not connected.</p>
    <details>
      <summary class="cursor-pointer text-sm">Camera GPIO assignments</summary>
      <div class="grid grid-cols-2 gap-3 mt-3">
        <label class="form-control block text-xs">Power enable<input type="number" min="0" max="48" step="1" required class="input input-sm input-bordered w-full" bind:value={config.cameraPowerGpio} /></label>
        <label class="form-control block text-xs">XCLK<input type="number" min="0" max="48" step="1" required class="input input-sm input-bordered w-full" bind:value={config.cameraXclkGpio} /></label>
        <label class="form-control block text-xs">SCCB SDA<input type="number" min="0" max="48" step="1" required class="input input-sm input-bordered w-full" bind:value={config.cameraSdaGpio} /></label>
        <label class="form-control block text-xs">SCCB SCL<input type="number" min="0" max="48" step="1" required class="input input-sm input-bordered w-full" bind:value={config.cameraSclGpio} /></label>
        <label class="form-control block text-xs">D0<input type="number" min="0" max="48" step="1" required class="input input-sm input-bordered w-full" bind:value={config.cameraD0Gpio} /></label>
        <label class="form-control block text-xs">D1<input type="number" min="0" max="48" step="1" required class="input input-sm input-bordered w-full" bind:value={config.cameraD1Gpio} /></label>
        <label class="form-control block text-xs">D2<input type="number" min="0" max="48" step="1" required class="input input-sm input-bordered w-full" bind:value={config.cameraD2Gpio} /></label>
        <label class="form-control block text-xs">D3<input type="number" min="0" max="48" step="1" required class="input input-sm input-bordered w-full" bind:value={config.cameraD3Gpio} /></label>
        <label class="form-control block text-xs">D4<input type="number" min="0" max="48" step="1" required class="input input-sm input-bordered w-full" bind:value={config.cameraD4Gpio} /></label>
        <label class="form-control block text-xs">D5<input type="number" min="0" max="48" step="1" required class="input input-sm input-bordered w-full" bind:value={config.cameraD5Gpio} /></label>
        <label class="form-control block text-xs">D6<input type="number" min="0" max="48" step="1" required class="input input-sm input-bordered w-full" bind:value={config.cameraD6Gpio} /></label>
        <label class="form-control block text-xs">D7<input type="number" min="0" max="48" step="1" required class="input input-sm input-bordered w-full" bind:value={config.cameraD7Gpio} /></label>
        <label class="form-control block text-xs">VSYNC<input type="number" min="0" max="48" step="1" required class="input input-sm input-bordered w-full" bind:value={config.cameraVsyncGpio} /></label>
        <label class="form-control block text-xs">HREF<input type="number" min="0" max="48" step="1" required class="input input-sm input-bordered w-full" bind:value={config.cameraHrefGpio} /></label>
        <label class="form-control block text-xs">PCLK<input type="number" min="0" max="48" step="1" required class="input input-sm input-bordered w-full" bind:value={config.cameraPclkGpio} /></label>
      </div>
    </details>
    <p class="text-xs text-base-content/60">Snapshot: /camera.jpg · MJPEG: port 81 /stream. Disabling the camera releases its GPIOs after restart.</p>
  </section>
</div>
