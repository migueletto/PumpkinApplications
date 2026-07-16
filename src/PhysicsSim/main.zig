const std = @import("std");
const pi = std.math.pi;
const pi2 = 2 * pi;

const pumpkin = @import("pumpkin");
const c = pumpkin.c;
const Win = pumpkin.Win;
const Frm = pumpkin.Frm;
const Ctl = pumpkin.Ctl;
const Mem = pumpkin.Mem;
const Dm = pumpkin.Dm;
const MemHandle = pumpkin.MemHandle;
const WindowType = pumpkin.WindowType;
const ControlType = pumpkin.ControlType;
const RectangleType = pumpkin.RectangleType;
const PointType = pumpkin.PointType;

const space = @import("space");
const vg = @import("vg");

const mainForm: u16 = 1000;
const aboutCmd: u16 = 1;
const paramsCmd: u16 = 2;
const dropButton: u16 = 2000;
const okButton: u16 = 2000;

const svgRsc: u32 = 0x74535647; // tSVG
const VectorGraphicsType = opaque {};

const groundY: f64 = 40.0;

const Simulation = struct {
  svgHandle: MemHandle = undefined,
  svgText: *void = undefined,
  vg: *VectorGraphicsType = undefined,
  space: space.Space2D = undefined,
  ground: space.SpaceBody = undefined,
  ball: space.SpaceBody = undefined,
  spaceWidth: f64 = 0,
  spaceHeight: f64 = 0,
  ballRadius: f64 = 0,
  i: u32 = 0,
  running: bool = false,
  buffer: *WindowType = undefined,
  background: *WindowType = undefined,

  oldRect: RectangleType = undefined,
  newRect: RectangleType = undefined,

  groundElasticity: f64 = 0.5,
  groundFriction: f64 = 0.4,
  ballElasticity: f64 = 0.8,
  ballFriction: f64 = 0.6,
  ballVelocity: f64 = 50.0,
};

var sim = Simulation {};

fn controlHandler(event: *pumpkin.EventType) bool {
  if (event.data.ctlSelect.controlID == dropButton) {
    if (!sim.running) {
      sim.ground.setElasticity(sim.groundElasticity);
      sim.ground.setFriction(sim.groundFriction);
      sim.ball.setElasticity(sim.ballElasticity);
      sim.ball.setFriction(sim.ballFriction);
      sim.ball.setPosition(sim.ballRadius, sim.spaceHeight - (sim.ballRadius + 32));
      sim.ball.setVelocity(sim.ballVelocity, 0);
      sim.running = true;
      sim.i = 0;
    } else {
      const prev: u16 = c.WinSetCoordinateSystem(144);
      c.WinCopyWindow(sim.background, c.WinGetActiveWindow(), null, 0, 0);
      _ = c.WinSetCoordinateSystem(prev);
      sim.running = false;
      sim.ball.setPosition(sim.ballRadius, sim.spaceHeight - (sim.ballRadius + 32));
      iterateSimulation(0.0);
    }
  }
  return false;
}

fn menuHandler(event: *pumpkin.EventType) bool {
  if (event.data.menu.itemID == aboutCmd) {
    pumpkin.Abt.showAboutPumpkin();
    return true;
  }
  if (event.data.menu.itemID == paramsCmd and !sim.running) {
    const formP = Frm.initForm(1001);
    Frm.setFieldNum(formP.?, 1001, sim.ballElasticity);
    Frm.setFieldNum(formP.?, 1002, sim.ballFriction);
    Frm.setFieldNum(formP.?, 1003, sim.ballVelocity);
    Frm.setFieldNum(formP.?, 1004, sim.groundElasticity);
    Frm.setFieldNum(formP.?, 1005, sim.groundFriction);
    if (Frm.doDialog(formP.?) == okButton) {
      sim.ballElasticity   = Frm.getFieldNum(formP.?, 1001);
      sim.ballFriction     = Frm.getFieldNum(formP.?, 1002);
      sim.ballVelocity     = Frm.getFieldNum(formP.?, 1003);
      sim.groundElasticity = Frm.getFieldNum(formP.?, 1004);
      sim.groundFriction   = Frm.getFieldNum(formP.?, 1005);
    }
    Frm.deleteForm(formP.?);
    return true;
  }
  return false;
}

fn idle() bool {
  if (sim.running) {
    iterateSimulation(10.0 / 60.0);
    sim.i += 1;
  }

  return false;
}

fn simpleFrmOpenHandler() bool {
  const formP = pumpkin.Frm.getActiveForm();
  pumpkin.Frm.drawForm(formP);
  var width: i16 = 0;
  var height: i16 = 0;
  Win.dimensions(&width, &height);
  var rect = RectangleType {
    .topLeft = PointType { .x = 0, .y = 15 },
    .extent  = PointType { .x = width, .y = 21 },
  };
  const old = c.WinSetForeColor(36);
  c.WinPaintRectangle(&rect, 0);
  _ = c.WinSetForeColor(old);
  const dy: i16 = @intFromFloat(groundY / 2.0);
  c.WinPaintLine(0, height - dy, width - 1, height - dy);
  initSimulation();
  return true;
}

fn mainFormEventHandler(event: *pumpkin.EventType) bool {
  return switch (event.eType) {
    pumpkin.eventTypes.frmOpen   => simpleFrmOpenHandler(),
    pumpkin.eventTypes.ctlSelect => controlHandler(event),
    pumpkin.eventTypes.menu      => menuHandler(event),
    pumpkin.eventTypes.nil       => idle(),
    else => false,
  };
}

// Space:
// X increases to the right
// Y increases upward
// Rotations are counter-clockwise and are in radians
// x and y positions represent the center of an object

fn initSimulation() void {
  const svgHandle = Dm.getResource(svgRsc, 1000);
  const svgText = Mem.handleLock(svgHandle);
  const svgLen = Mem.handleSize(svgHandle);
  const vgt: *VectorGraphicsType = @ptrCast(vg.VgCreate(svgText, svgLen));
  vg.VgScale(vgt, 0.15);
  vg.VgFreeze(vgt);
  var vgWidth: f64 = 0;
  var vgHeight: f64 = 0;
  vg.VgSize(vgt, &vgWidth, &vgHeight);
  const ballRadius: f64 = vgWidth / 2.0;

  const prev: u16 = c.WinSetCoordinateSystem(144);
  var width: i16 = 0;
  var height: i16 = 0;
  Win.dimensions(&width, &height);
  const spaceWidth: f64 = @floatFromInt(width);
  const spaceHeight: f64 = @floatFromInt(height);

  var s: space.Space2D = space.init();
  s.setGravity(0, -100);
  const ground = s.addSegment(0, groundY, spaceWidth, groundY, -1);
  const ball = s.addCircle(ballRadius, 5);
  var err: u16 = 0;

  sim = Simulation {
    .svgHandle = svgHandle,
    .svgText = svgText,
    .vg = vgt,
    .space = s,
    .ground = ground,
    .ball = ball,
    .spaceWidth = spaceWidth,
    .spaceHeight = spaceHeight,
    .ballRadius = ballRadius,
    .i = 0,
    .oldRect = RectangleType {
      .topLeft = PointType { .x = 0, .y = 0 },
      .extent  = PointType { .x = @intFromFloat(vgWidth), .y = @intFromFloat(vgHeight) },
    },
    .newRect = RectangleType {
      .topLeft = PointType { .x = 0, .y = 0 },
      .extent  = PointType { .x = @intFromFloat(vgWidth), .y = @intFromFloat(vgHeight) },
    },
    .buffer = @ptrCast(c.WinCreateOffscreenWindow(width, height, @intFromEnum(pumpkin.windowFormats.native), &err)),
    .background = @ptrCast(c.WinCreateOffscreenWindow(width, height, @intFromEnum(pumpkin.windowFormats.native), &err)),
  };

  c.WinCopyWindow(c.WinGetActiveWindow(), sim.background, null, 0, 0);
  _ = c.WinSetCoordinateSystem(prev);

  sim.ball.setPosition(sim.ballRadius, sim.spaceHeight - (sim.ballRadius + 32));
  iterateSimulation(0.0);
}

fn iterateSimulation(dt: f64) void {
  sim.space.step(dt);
  var x: f64 = 0;
  var y: f64 = 0;
  sim.ball.getPosition(&x, &y);
  y = sim.spaceHeight - y;
  var angle = sim.ball.getAngle();
  while (angle < 0) { angle += pi2; }
  while (angle >= pi2) { angle -= pi2; }
  vg.VgRotate(sim.vg, pi2 - angle, 200, 200);
  pumpkin.debug(pumpkin.DEBUG_INFO, "test", "{d:.2}: x = {d:.2}, y = {d:.2}, angle = {d:.2}", .{ sim.i, x, y, angle });

  const ballX: i16 = @intFromFloat(x - sim.ballRadius);
  const ballY: i16 = @intFromFloat(y - sim.ballRadius);

  sim.newRect.topLeft.x = ballX;
  sim.newRect.topLeft.y = ballY;
  var rect: RectangleType = undefined;
  c.RctGetUnion(&sim.oldRect, &sim.newRect, &rect);

  const prev: u16 = c.WinSetCoordinateSystem(144);
  c.WinCopyWindow(sim.background, sim.buffer, &rect, rect.topLeft.x, rect.topLeft.y);
  const old: *WindowType = @ptrCast(c.WinSetDrawWindow(sim.buffer));
  vg.VgRender(sim.vg, ballX, ballY);
  _ = c.WinSetDrawWindow(old);
  c.WinCopyWindow(sim.buffer, c.WinGetActiveWindow(), &rect, rect.topLeft.x, rect.topLeft.y);
  _ = c.WinSetCoordinateSystem(prev);

  sim.oldRect.topLeft.x = ballX;
  sim.oldRect.topLeft.y = ballY;
}

fn deinitSimulation() void {
  sim.space.deinit();

  vg.VgDestroy(sim.vg);
  Mem.handleUnlock(sim.svgHandle);
  _ = Dm.releaseResource(sim.svgHandle);

  c.WinDeleteWindow(sim.background, 0);
  c.WinDeleteWindow(sim.buffer, 0);
}

export fn PilotMain(cmd: c_ushort, cmdPBP: *void, launchFlags: c_ushort) c_uint {
  const launchCode: pumpkin.launchCodes = @enumFromInt(cmd);
  _ = cmdPBP;
  _ = launchFlags;

  if (launchCode == pumpkin.launchCodes.normalLaunch) {
    var map = Frm.FormMap.init(pumpkin.PumpkinAllocator);
    defer map.deinit();
    map.put(mainForm, mainFormEventHandler) catch { return 0; };
    Frm.normalLaunchMain(mainForm, &map, 10);
    deinitSimulation();
  }

  return 0;
}
