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

    lib.addIncludePath(b.path("../../../PumpkinOS/src/libpumpkin"));
    lib.addIncludePath(b.path("."));
    lib.addLibraryPath(b.path("../../../PumpkinOS/bin"));
    lib.linkSystemLibrary("pit");
    lib.linkSystemLibrary("pumpkin");
    lib.linkSystemLibrary("chipmunk");
    lib.linkSystemLibrary("pluto");
    lib.linkLibC();
    b.installArtifact(lib);
}
