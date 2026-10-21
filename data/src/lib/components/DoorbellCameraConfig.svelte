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
  <section class="p-3 bg-base-100 rounded-lg space-y-3">
    <h3 class="font-semibold">HTTP notifications / Loxone</h3>
    <label class="flex items-center gap-3"><input type="checkbox" class="toggle toggle-primary toggle-sm" bind:checked={config.httpNotifyEnabled} />Enable HTTP notifications</label>
    <p class="text-xs text-base-content/60">Sends HTTP GET requests directly, without MQTT. Each blank URL disables that event. Use a pulse command for the doorbell and set-value commands for lock states.</p>
    <label class="form-control block text-sm">Doorbell press URL<input type="url" maxlength="512" class="input input-sm input-bordered w-full" placeholder="http://miniserver/…" bind:value={config.httpDoorbellUrl} /></label>
    <label class="form-control block text-sm">Locked URL<input type="url" maxlength="512" class="input input-sm input-bordered w-full" placeholder="http://miniserver/…" bind:value={config.httpLockedUrl} /></label>
    <label class="form-control block text-sm">Unlocked URL<input type="url" maxlength="512" class="input input-sm input-bordered w-full" placeholder="http://miniserver/…" bind:value={config.httpUnlockedUrl} /></label>
    <label class="form-control block text-sm">HTTP Basic username (optional)<input type="text" maxlength="128" autocomplete="off" class="input input-sm input-bordered w-full" bind:value={config.httpUsername} /></label>
    <label class="form-control block text-sm">HTTP Basic password<input type="password" maxlength="128" autocomplete="new-password" class="input input-sm input-bordered w-full" bind:value={config.httpPassword} /></label>
    <p class="text-xs text-base-content/60">Credentials are shared by all three URLs; use endpoints on the same trusted server. HTTP transmits credentials without encryption. HTTPS requires a trusted server certificate. Redirects and automatic retries are disabled. Failed events are logged; delivery is best effort.</p>
    <p class="text-xs text-base-content/60">Lock notifications follow the firmware's current state, not just the requested target. Physical confirmation requires lock feedback. To stop MQTT, clear its broker address in MQTT settings.</p>
  </section>
</div>
