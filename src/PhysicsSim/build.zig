const std = @import("std");

pub fn build(b: *std.Build) void {
    const target = b.standardTargetOptions(.{});
    const optimize = b.standardOptimizeOption(.{});

    const pumpkin = b.addModule("pumpkin", .{
        .root_source_file = b.path("../../../PumpkinOS/src/libpumpkin/pumpkin.zig"),
        .target = target,
    });

    pumpkin.addIncludePath(b.path("../../../PumpkinOS/src/libpumpkin"));

    const space = b.addModule("space", .{
        .root_source_file = b.path("../../../PumpkinOS/src/libchipmunk/space.zig"),
        .target = target,
    });

    space.addIncludePath(b.path("../../../PumpkinOS/src/libchipmunk/chipmunk"));

    const lib = b.addLibrary(.{
        .name = "PhysicsSim",
        .linkage = .dynamic,
        .root_module = b.createModule(.{
            .root_source_file = b.path("main.zig"),
            .target = target,
            .optimize = optimize,
            .imports = &.{
                .{ .name = "pumpkin", .module = pumpkin },
                .{ .name = "space",   .module = space },
            },
        }),
    });

    lib.root_module.addIncludePath(b.path("../../../PumpkinOS/src/libpumpkin"));
    lib.root_module.addIncludePath(b.path("."));
    lib.root_module.addLibraryPath(b.path("../../../PumpkinOS/bin"));
    lib.root_module.linkSystemLibrary("pit", .{});
    lib.root_module.linkSystemLibrary("pumpkin", .{});
    lib.root_module.linkSystemLibrary("chipmunk", .{});
    lib.root_module.linkSystemLibrary("pluto", .{});
    b.installArtifact(lib);
}
