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
  { name: "charibert", value: "charibert" },
  { name: "kawatea_block", value: "kawatea_block" },
  { name: "kawatea_block2", value: "kawatea_block2" },
  { name: "kawatea_block_iterate", value: "kawatea_block_iterate" },
  { name: "kawatea_no_block", value: "kawatea_no_block" },
  { name: "kenkoooo-1e3", value: "kenkoooo-1e3" },
  { name: "kenkoooo-1e5", value: "kenkoooo-1e5" },
  { name: "kenkoooo-sa", value: "kenkoooo-sa" },
  { name: "kenkoooo-sa-improve", value: "kenkoooo-sa-improve" },
  { name: "kenkoooo-swap", value: "kenkoooo-swap" },
  { name: "mkut_hand", value: "mkut_hand" },
  { name: "mkut_rect", value: "mkut_rect" },
  { name: "synced-bests", value: "synced-bests" },
  { name: "thordan", value: "thordan" },
  { name: "yuusti_hand", value: "yuusti_hand" },
  { name: "zephirin", value: "zephirin" },
];
