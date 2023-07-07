import Canvas from "@/components/visualizer";
import useProblem from "./hooks/useProblem";
import useSolution from "./hooks/useSolution";

type Props = {};
const Visualizer = ({}: Props) => {
  const problem = useProblem(42);
  const solution = useSolution("thordan/42");
  if (problem.isLoading) {
    return <p>loading...</p>;
  }
  if (problem.error) {
    return <p>{JSON.stringify(problem.error)}</p>;
  }
  if (!problem.data) {
    return <p>undefined</p>;
  }

  return <Canvas problem={problem.data} solution={solution.data} />;
};
export default Visualizer;
