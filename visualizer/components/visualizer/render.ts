type Problem = {
  readonly room: {
    readonly width: number;
    readonly height: number;
  };
  readonly stage: {
    readonly width: number;
    readonly height: number;
    readonly bottom: number;
    readonly left: number;
  };
  readonly musicians: number[];
  readonly attendees: {
    readonly x: number;
    readonly y: number;
    readonly tastes: number[];
  }[];
};

type Solution = {
  placements: {
    x: number;
    y: number;
  }[];
};

export const render = (
  ctx: CanvasRenderingContext2D,
  problem: Problem,
  solution?: Solution
) => {
  const widthZoom = ctx.canvas.width / problem.room.width;
  const heightZoom = ctx.canvas.height / problem.room.height;
  const zoom = Math.min(widthZoom, heightZoom);
  const { room, stage, attendees } = applyZoom(problem, zoom);

  const personSize = zoom * 2;
  const musicianSize = zoom * 10;

  ctx.fillStyle = "lightyellow";
  ctx.fillRect(0, 0, room.width, room.height);

  ctx.fillStyle = "white";
  ctx.fillRect(stage.left, stage.bottom, stage.width, stage.height);

  ctx.fillStyle = "black";
  ctx.strokeRect(stage.left, stage.bottom, stage.width, stage.height);

  attendees.forEach((attendee) => {
    ctx.fillStyle = "green";
    ctx.beginPath();
    ctx.arc(attendee.x, attendee.y, personSize, 0, 2 * Math.PI);
    ctx.fill();
  });

  solution?.placements.forEach((musician) => {
    ctx.fillStyle = "blue";
    ctx.beginPath();
    ctx.arc(musician.x * zoom, musician.y * zoom, personSize, 0, 2 * Math.PI);
    ctx.fill();

    ctx.globalAlpha = 0.3;
    ctx.beginPath();
    ctx.arc(musician.x * zoom, musician.y * zoom, musicianSize, 0, 2 * Math.PI);
    ctx.fill();
    ctx.globalAlpha = 1.0;
  });
};

const applyZoom = (problem: Problem, zoom: number): Problem => {
  return {
    room: {
      width: problem.room.width * zoom,
      height: problem.room.height * zoom,
    },
    stage: {
      width: problem.stage.width * zoom,
      height: problem.stage.height * zoom,
      bottom: problem.stage.bottom * zoom,
      left: problem.stage.left * zoom,
    },
    musicians: [...problem.musicians],
    attendees: problem.attendees.map((attendee) => ({
      x: attendee.x * zoom,
      y: attendee.y * zoom,
      tastes: [...attendee.tastes],
    })),
  };
};
