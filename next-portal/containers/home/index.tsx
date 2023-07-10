import Home from "@/components/home";
import useBestSolutions from "./hooks";
import { useState } from "react";
import useTags from "./hooks/useTags";

function normalize(tag: string): string | undefined {
  if (tag.trim() == '(all)') {
    return undefined;
  }
  return tag;
}

const HomeContainer = () => {
  const [tag, setTag] = useState('(all)');
  const problems = useBestSolutions(normalize(tag));
  const tags = useTags();

  if (problems.error) {
    return <p>{JSON.stringify(problems.error)}</p>;
  }
  if (!problems.data) {
    return <p>Loading...</p>;
  }

  return (
    <div>
      Tag: <select value={tag} onChange={(e) => setTag(e.currentTarget.value)}>
        <option value="(all)">(all)</option>
        {tags.data?.tags?.map((t) => <option key={t} value={t}>{t}</option>)}
      </select>
      <Home problems={problems.data} />
    </div>
  );
};

export default HomeContainer;
