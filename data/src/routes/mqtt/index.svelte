<script lang="ts">
	import AppMqtt from '$lib/components/AppMqtt.svelte';
	import { route } from 'sv-router/generated';
</script>

<AppMqtt mqtt={route.meta.mqttData?.mqtt} misc={route.meta.mqttData?.misc} error={route.meta.mqttData?.error} />