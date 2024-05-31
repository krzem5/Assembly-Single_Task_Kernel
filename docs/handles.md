# Handle usage by type

## Kernel

| Type | Owned handles |
|--|--|
| `aml_bus_device_t` | itself, parent |
| `drive_t` | itself, `partition_table_descriptor_t` |
| `elf_loader_context_t` | `process_t`, `thread_t` |
| `event_t` | itself |
| `fd_iterator_t` | itself, `vfs_node_t` (via rc) |
| `fd_t` | itself, `vfs_node_t` (via rc) |
| `filesystem_descriptor_t` | itself |
| `filesystem_t` | itself, `filesystem_descriptor_t`, `partition_t`, root `vfs_node_t` (via rc) |
| `handle_descriptor_t` | itself |
| `keyring_t` | itself |
| `module_t` | itself |
| `mutex_t` | `thread_t` |
| `network_layer1_device_t` | itself |
| `omm_allocator_t` | itself |
| `partition_t` | itself, `drive_t`, `partition_table_descriptor_t` |
| `partition_table_descriptor_t` | itself |
| `pci_device_t` | itself |
| `pmm_counter_descriptor_t` | itself |
| `process_t` | itself, parent |
| `thread_t` | itself, `process_t` (via shared thread list) |
| `timer_t` | itself |

> [!CAUTION]
> USB subsystem has not been done yet!

## Modules

| Type | Owned handles |
|--|--|
| `opengl_driver_instance_t` | itself |
| `opengl_state_t` | itself, `opengl_driver_instance_t`, `ui_framebuffer_t` |
| `ui_display_t` | itself |
| `ui_framebuffer_t` | itself, `ui_display_t` |
| `virgl_opengl_buffer_t` | itself |
| `virgl_opengl_sampler_t` | itself |
| `virgl_opengl_shader_t` | itself |
| `virgl_opengl_texture_t` | itself |
| `virgl_opengl_vertex_array_t` | itself |
| `virtio_device_t` | itself |
