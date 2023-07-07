import Canvas from "@/components/visualizer";
import useProblem from "./hooks/useProblem";

type Props = {};
const Visualizer = ({}: Props) => {
  const problem = useProblem(1);
  if (problem.isLoading) {
    return <p>loading...</p>;
  }
  if (problem.error) {
    return <p>{JSON.stringify(problem.error)}</p>;
  }
  if (!problem.data) {
    return <p>undefined</p>;
  }
  return <Canvas problem={problem.data} />;
};
export default Visualizer;
