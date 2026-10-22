import type { Hooks } from 'sv-router';
import type { MqttConfig, MiscConfig } from '$lib/types/api';
import { setLoadingState } from '$lib/stores/system.svelte';

declare module 'sv-router' {
  interface RouteMeta {
    mqttData?: { mqtt: MqttConfig | null; misc: MiscConfig | null; error: string | null };
  }
}

export default {
  async beforeLoad({ meta }) {
    try {
      setLoadingState(true);
      const [res, misc] = await Promise.all([fetch('/config?type=mqtt').then(r => r.json()), fetch('/config?type=misc').then(r => r.json())]);
      if (!misc.success) throw new Error(misc.error);
      if (!res.success) throw new Error(res.error);
      meta.mqttData = { mqtt: res.data as MqttConfig, misc: misc.data as MiscConfig, error: null };
    } catch (error) {
      console.error('Failed to load mqtt:', error);
      meta.mqttData = { mqtt: null, misc: null, error: error instanceof Error ? error.message : 'Unknown error' };
    } finally {
      setLoadingState(false);
    }
  },
} satisfies Hooks;
