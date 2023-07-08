import { useMemo } from "react";
import useProblems from "./useProblems";
import useSolutions from "./useSolutions";

const useBestSolutions = () => {
  const problems = useProblems();
  const solutions = useSolutions();

  const data = useMemo(() => {
    const bestSolutions = new Map<
      number,
      {
        solverName: string;
        problemId: number;
        score: number;
        solutionPath: string;
      }
    >();
    solutions.data?.solutions.forEach((solution) => {
      const current = bestSolutions.get(solution.problemId);
      if (!current || solution.score > current.score) {
        bestSolutions.set(solution.problemId, solution);
      }
    });

    return problems.data?.problems.map((problem) => {
      const solution = bestSolutions.get(problem.problemId);
      return {
        ...problem,
        bestSolution: solution
          ? {
            name: solution.solverName,
            score: solution.score,
            solutionPath: solution.solutionPath,
          }
          : null,
      };
    });
  }, [problems.data?.problems, solutions.data?.solutions]);
  const error = problems.error || solutions.error;
  return {
    error,
    data,
  };
};
export default useBestSolutions;
