import Canvas from "@/components/visualizer";
import useProblem from "./hooks/useProblem";
import useSolution from "./hooks/useSolution";
import { useCallback, useState } from "react";

type Props = {};
const Visualizer = ({}: Props) => {
  const [selected, setSelected] = useState(1);
  const [solver, setSolver] = useState<string>();
  const setProblemId = useCallback((problemId: number) => {
    setSelected(problemId);
    setSolver(undefined);
  }, []);
  const problem = useProblem(selected);
  const solution = useSolution(solver, selected);

  return (
    <div style={{ display: "flex" }}>
      {problem.data && (
        <Canvas problem={problem.data} solution={solution.data} />
      )}
      <div style={{ width: "100px" }}>
        <select
          value={selected}
          onChange={(e) => setProblemId(Number.parseInt(e.target.value))}
        >
          {Array.from({ length: 45 }).map((_, i) => (
            <option key={i} value={i + 1}>
              {i + 1}
            </option>
          ))}
        </select>
      </div>
      <div>
        <select
          value={solver ?? "none"}
          onChange={(e) =>
            setSolver(e.target.value === "none" ? undefined : e.target.value)
          }
        >
          {SOLVERS.map(({ name, value }) => (
            <option key={name} value={value}>
              {name}
            </option>
          ))}
        </select>
      </div>
    </div>
  );
};
export default Visualizer;

const SOLVERS = [
  {
    name: "select solver",
    value: "none",
  },
  {
    name: "thordan",
    value: "thordan",
  },
];
