const std = @import("std");

pub fn build(b: *std.Build) void {
    const target = b.standardTargetOptions(.{});
    const optimize = b.standardOptimizeOption(.{});

    const translate_c = b.addTranslateC(.{
        .root_source_file = b.path("../../../PumpkinOS/src/libpumpkin/zigpumpkin.h"),
        .target = target,
        .optimize = optimize,
    });

    const translate_ch = b.addTranslateC(.{
        .root_source_file = b.path("../../../PumpkinOS/src/libchipmunk/chipmunk/chipmunk.h"),
        .target = target,
        .optimize = optimize,
    });

    const translate_vg = b.addTranslateC(.{
        .root_source_file = b.path("./vg.h"),
        .target = target,
        .optimize = optimize,
    });

    const pumpkin = b.addModule("pumpkin", .{
        .root_source_file = b.path("../../../PumpkinOS/src/libpumpkin/pumpkin.zig"),
        .target = target,
        .imports = &.{
            .{ .name = "c", .module = translate_c.createModule(), },
        },
    });

    const space = b.addModule("space", .{
        .root_source_file = b.path("../../../PumpkinOS/src/libchipmunk/space.zig"),
        .target = target,
        .imports = &.{
            .{ .name = "ch", .module = translate_ch.createModule(), },
        },
    });

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
                .{ .name = "vg",      .module = translate_vg.createModule(), },
            },
        }),
    });

    lib.root_module.addLibraryPath(b.path("../../../PumpkinOS/bin"));
    lib.root_module.linkSystemLibrary("pit", .{});
    lib.root_module.linkSystemLibrary("pumpkin", .{});
    lib.root_module.linkSystemLibrary("chipmunk", .{});
    lib.root_module.linkSystemLibrary("pluto", .{});
    b.installArtifact(lib);
}
