type Props = {
  problem: Problem;
  solution: Solution | null;
};
const Svg = ({ problem, solution }: Props) => {
  return (
    <svg
      width="1000"
      height="1000"
      viewBox={`0 0 ${problem.room.width} ${problem.room.height}`}
    >
      <rect
        x={0}
        y={0}
        width={problem.room.width}
        height={problem.room.height}
        fill="lightyellow"
        stroke="none"
      />
      <rect
        x={problem.stage.left}
        y={problem.stage.bottom}
        width={problem.stage.width}
        height={problem.stage.height}
        fill="white"
        stroke="black"
      />
      {problem.attendees.map(({ x, y }, i) => (
        <circle key={i} cx={x} cy={y} r="5" fill="green" stroke="none" />
      ))}
      {solution?.placements.map(({ x, y }, i) => (
        <circle key={i} cx={x} cy={y} r="5" fill="blue" stroke="none" />
      ))}
    </svg>
  );
};

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

export default Svg;
